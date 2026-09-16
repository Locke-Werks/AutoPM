// AutoPM as an MCP server.
//
// The same core the window uses, with a different face on it, following
// ProjectMan's shape. It exists because the record and the conversation were
// two separate places the same decisions got written: a decision made in chat
// had to be retyped into the tool, and usually was not.
//
// The provenance rule is enforced here rather than suggested. A write that
// claims something was specified or agreed must carry its evidence, or it is
// refused. An agent cannot put words in anybody's mouth through this API.
#include "core/coach.h"
#include "core/gitlog.h"
#include "core/workspace.h"
#include "json.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <shlobj.h>
#include <windows.h>
#endif

namespace {

pm::Workspace g_workspace;
std::string g_recordsDir;

// ── where things live ────────────────────────────────────────────────────

std::string executableDir() {
#ifdef _WIN32
    char path[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path().string();
#else
    return std::filesystem::current_path().string();
#endif
}

// Documents redirects to OneDrive on this machine, so ask Windows rather than
// assuming a path under the profile.
std::string documentsDir() {
#ifdef _WIN32
    PWSTR wide = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &wide))) {
        const std::wstring path(wide);
        CoTaskMemFree(wide);
        return std::filesystem::path(path).string();
    }
#endif
    const char* home = std::getenv("USERPROFILE");
    return home ? (std::filesystem::path(home) / "Documents").string() : std::string(".");
}

// ── shaping replies ──────────────────────────────────────────────────────

json::Value textResult(const std::string& text, bool isError = false) {
    json::Object content;
    content["type"] = json::Value("text");
    content["text"] = json::Value(text);
    json::Object result;
    result["content"] = json::Value(json::Array{json::Value(content)});
    if (isError) result["isError"] = json::Value(true);
    return json::Value(result);
}

json::Value schema(json::Object properties, std::vector<std::string> required) {
    json::Object s;
    s["type"] = json::Value("object");
    s["properties"] = json::Value(std::move(properties));
    json::Array req;
    for (const std::string& name : required) req.push_back(json::Value(name));
    s["required"] = json::Value(std::move(req));
    return json::Value(s);
}

json::Value param(const std::string& type, const std::string& description) {
    json::Object p;
    p["type"] = json::Value(type);
    p["description"] = json::Value(description);
    return json::Value(p);
}

json::Value tool(const std::string& name, const std::string& description, json::Value input) {
    json::Object t;
    t["name"] = json::Value(name);
    t["description"] = json::Value(description);
    t["inputSchema"] = std::move(input);
    return json::Value(t);
}

// ── loading a project by name or id ──────────────────────────────────────

std::shared_ptr<pm::Record> openProject(const std::string& wanted, std::string& error) {
    for (const pm::ProjectSummary& summary : g_workspace.list()) {
        if (summary.id != wanted && summary.name != wanted) continue;
        return g_workspace.load(summary.path, error);
    }
    error = "no project called \"" + wanted + "\". Use autopm_projects to list them.";
    return nullptr;
}

bool saveProject(const std::shared_ptr<pm::Record>& record, std::string& error) {
    return record->save(record->path, error);
}

// The rule the whole product rests on, enforced at the boundary: a claim that
// somebody said or agreed something has to carry what they said.
bool provenanceIsUsable(const std::string& provenance, const std::string& evidence,
                        std::string& error) {
    const pm::Provenance parsed = pm::provenanceFromString(provenance);
    if (!provenance.empty() && pm::provenanceToString(parsed) != provenance) {
        error = "provenance must be one of specified, agreed, unobjected, untagged. Got \"" +
                provenance + "\".";
        return false;
    }
    if (pm::provenanceIsDecision(parsed) && evidence.empty()) {
        error = "provenance \"" + provenance +
                "\" needs evidence: the quote and date it rests on. Only specified and agreed "
                "count as decisions, and a decision with nothing behind it is the thing this "
                "tool exists to prevent. Use \"unobjected\" if you are proposing it.";
        return false;
    }
    return true;
}

std::string describeEntry(const std::string& key, const pm::Entry& entry) {
    std::string out = key;
    out += "  [" + pm::provenanceToString(entry.provenance) + "]";
    if (!entry.evidence.empty()) out += "  evidence: " + entry.evidence;
    out += "\n";
    if (!entry.value.empty()) out += entry.value + "\n";
    for (const pm::Row& row : entry.rows) {
        out += "  - (" + row.id + ")";
        if (row.provenance != pm::Provenance::Untagged)
            out += " [" + pm::provenanceToString(row.provenance) + "]";
        for (const auto& cell : row.cells) out += " " + cell.first + "=" + cell.second + ";";
        if (!row.evidence.empty()) out += " evidence: " + row.evidence;
        out += "\n";
    }
    return out;
}

// ── the tools ────────────────────────────────────────────────────────────

json::Value listTools() {
    json::Array tools;

    tools.push_back(tool(
        "autopm_projects",
        "List the AutoPM projects on this machine, with how far through the lifecycle each one "
        "is. Start here: every other tool takes a project name.",
        schema({}, {})));

    tools.push_back(tool(
        "autopm_create",
        "Start a new project. Writes the record and nothing else: the charter is then answered "
        "field by field like any other, because a project nobody has chartered is not a project. "
        "The accent is the project's own colour and is best taken from the house families: "
        "#B05CF6 violet, #3D7DFF blue, #FF2D95 magenta, #FF1E3C crimson, #FF5A2A ember, "
        "#2EE8FF cyan. Blue, ember and crimson also carry status meanings; magenta and cyan do "
        "not.",
        schema({{"name", param("string", "What the project is called.")},
                {"accent", param("string", "#rrggbb. Defaults to house violet.")},
                {"repo", param("string", "Path to the work tree whose history documents it. "
                                         "Optional, and read only ever for evidence.")}},
               {"name"})));

    tools.push_back(tool(
        "autopm_screens",
        "The screens and fields a project can hold, with what each field is for and what a good "
        "answer looks like. Read this before writing anything, so an entry lands in the field "
        "the PM would expect.",
        schema({{"screen", param("string", "One screen id to describe in full. Omit for the "
                                           "whole list.")}},
               {})));

    tools.push_back(tool(
        "autopm_read",
        "Read a project record: what it claims, where each claim came from, and what is still "
        "blank.",
        schema({{"project", param("string", "Project name or id.")},
                {"screen", param("string", "One screen id. Omit for the whole record.")}},
               {"project"})));

    tools.push_back(tool(
        "autopm_set",
        "Write a single-value field, with where the answer came from. Provenance is one of "
        "specified (they said it, quote them), agreed (they approved your summary), unobjected "
        "(you proposed it and nobody answered) or untagged. specified and agreed are refused "
        "without evidence.",
        schema({{"project", param("string", "Project name or id.")},
                {"field", param("string", "Field key, as screen.field, e.g. charter.purpose.")},
                {"value", param("string", "The answer.")},
                {"provenance", param("string", "specified | agreed | unobjected | untagged.")},
                {"evidence", param("string", "The quote and date this rests on.")}},
               {"project", "field", "value"})));

    tools.push_back(tool(
        "autopm_add_row",
        "Add a row to a table field: a decision, a risk, a change request, an issue, a board "
        "card, a lesson. Same provenance rule as autopm_set.",
        schema({{"project", param("string", "Project name or id.")},
                {"field", param("string", "Field key, e.g. issues.decisions or board.cards.")},
                {"cells", param("object", "Column id to value. Use autopm_screens to see the "
                                          "columns and their allowed values.")},
                {"provenance", param("string", "specified | agreed | unobjected | untagged.")},
                {"evidence", param("string", "The quote and date this row rests on.")}},
               {"project", "field", "cells"})));

    tools.push_back(tool(
        "autopm_update_row",
        "Change cells on an existing row: move a card, close a risk, answer a change request.",
        schema({{"project", param("string", "Project name or id.")},
                {"field", param("string", "Field key.")},
                {"row", param("string", "Row id, or the value of its ref column.")},
                {"cells", param("object", "Column id to value. Only the ones given change.")},
                {"provenance", param("string", "specified | agreed | unobjected | untagged.")},
                {"evidence", param("string", "The quote and date this rests on.")}},
               {"project", "field", "row", "cells"})));

    tools.push_back(tool(
        "autopm_review",
        "Run the coach over a project: which fields are blank, and which answers are thin. This "
        "is what a reviewer would ask about. Worth calling after writing.",
        schema({{"project", param("string", "Project name or id.")},
                {"screen", param("string", "One screen id. Omit for the whole record.")}},
               {"project"})));

    tools.push_back(tool(
        "autopm_git_activity",
        "What was actually committed in a date window, in the repository a project names. For "
        "evidence: pinning a record entry to a commit makes it checkable. Reports only; it "
        "never moves anything in the record.",
        schema({{"project", param("string", "Project name or id.")},
                {"since", param("string", "yyyy-mm-dd. Omit for no lower bound.")},
                {"until", param("string", "yyyy-mm-dd. Omit for no upper bound.")},
                {"limit", param("number", "Maximum commits. Default 60.")}},
               {"project"})));

    tools.push_back(tool(
        "autopm_reconcile",
        "Hold the record up against the repository: for each sprint, what the board claims was "
        "finished against what was actually committed in that window. Finds sprints that claim "
        "work with no commits behind them, and commits with no sprint that claims them. Reports "
        "only; deciding what it means is the PM's job.",
        schema({{"project", param("string", "Project name or id.")}}, {"project"})));

    json::Object result;
    result["tools"] = json::Value(std::move(tools));
    return json::Value(result);
}

// ── calling them ─────────────────────────────────────────────────────────

json::Value callProjects() {
    const std::vector<pm::ProjectSummary> projects = g_workspace.list();
    if (projects.empty())
        return textResult("No projects yet. They live in " + g_recordsDir + ".");

    std::string out = "Projects in " + g_recordsDir + ":\n\n";
    for (const pm::ProjectSummary& summary : projects) {
        std::string error;
        auto record = g_workspace.load(summary.path, error);
        int fields = 0, answered = 0, notes = 0;
        if (record) {
            for (const pm::Screen& screen : g_workspace.definitions().screens()) {
                const pm::Progress p = pm::progressOf(screen, *record);
                fields += p.fields;
                answered += p.answered;
                notes += p.notes;
            }
        }
        out += summary.name + "  (id " + summary.id + ")\n";
        out += "  phase " + (summary.phase.empty() ? std::string("unset") : summary.phase) +
               ", saved " + summary.modified + "\n";
        out += "  " + std::to_string(answered) + " of " + std::to_string(fields) +
               " fields answered, " + std::to_string(notes) + " thin\n";
        if (record && !record->repo.empty()) out += "  repo " + record->repo + "\n";
        out += "\n";
    }
    return textResult(out);
}

json::Value callCreate(const json::Value& args) {
    const std::string name = args.str("name");
    if (name.empty()) return textResult("a project needs a name.", true);
    for (const pm::ProjectSummary& summary : g_workspace.list()) {
        if (summary.name == name)
            return textResult("there is already a project called \"" + name + "\".", true);
    }

    std::string accent = args.str("accent", "#B05CF6");
    if (accent.size() != 7 || accent[0] != '#')
        return textResult("accent must be #rrggbb. Got \"" + accent + "\".", true);

    const std::string repo = args.str("repo");
    if (!repo.empty() && !pm::Git::isRepository(repo))
        return textResult(repo + " is not a git work tree.", true);

    std::string error;
    auto record = g_workspace.create(name, accent, error);
    if (!record) return textResult(error, true);
    if (!repo.empty()) {
        record->repo = repo;
        if (!record->save(record->path, error)) return textResult(error, true);
    }

    std::string out = "Created " + record->name + " (id " + record->id + ") at " +
                      record->path + ".\nPhase " + record->phase + ", accent " + accent + ".\n";
    if (!repo.empty()) out += "Repo " + repo + ".\n";
    out += "\nEvery field is blank. The charter comes first: autopm_screens charter lists "
           "what it asks and why.\n";
    return textResult(out);
}

json::Value callScreens(const json::Value& args) {
    const std::string wanted = args.str("screen");
    std::string out;
    for (const pm::Screen& screen : g_workspace.definitions().screens()) {
        if (!wanted.empty() && screen.id != wanted) continue;
        out += screen.id + "  —  " + screen.title + "  (" + screen.phase + ")\n";
        if (!screen.process.empty()) out += "  " + screen.process + "\n";
        if (wanted.empty()) {
            for (const pm::Field& field : screen.fields)
                out += "    " + screen.id + "." + field.id + "  " + field.label + "  [" +
                       field.type + "]\n";
            out += "\n";
            continue;
        }
        out += "\n" + screen.intro + "\n\n";
        for (const pm::Field& field : screen.fields) {
            out += "--- " + screen.id + "." + field.id + "  (" + field.type +
                   (field.origin == "yours" ? ", not from the standard" : "") + ")\n";
            out += field.label + "\n";
            if (!field.prompt.empty()) out += "Asks: " + field.prompt + "\n";
            if (!field.why.empty()) out += "Why: " + field.why + "\n";
            if (!field.ifBlank.empty()) out += "If blank: " + field.ifBlank + "\n";
            if (!field.example.empty()) out += "Example: " + field.example + "\n";
            for (const pm::Column& column : field.columns) {
                out += "  column " + column.id + " (" + column.label + ", " + column.type + ")";
                if (!column.options.empty()) {
                    out += " one of:";
                    for (const pm::Option& option : column.options) out += " " + option.value + ";";
                }
                out += "\n";
            }
            out += "\n";
        }
    }
    if (out.empty()) return textResult("No screen called \"" + wanted + "\".", true);
    return textResult(out);
}

json::Value callRead(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);

    const std::string wanted = args.str("screen");
    std::string out = record->name + "  (phase " + record->phase + ")\n";
    if (!record->repo.empty()) out += "repo: " + record->repo + "\n";
    out += "\n";

    for (const pm::Screen& screen : g_workspace.definitions().screens()) {
        if (!wanted.empty() && screen.id != wanted) continue;
        out += "== " + screen.title + " (" + screen.id + ")\n";
        for (const pm::Field& field : screen.fields) {
            const std::string key = screen.id + "." + field.id;
            const pm::Entry* entry = record->entry(key);
            if (!entry || entry->isEmpty()) {
                out += key + "  BLANK — " + field.ifBlank + "\n";
                continue;
            }
            out += describeEntry(key, *entry);
        }
        out += "\n";
    }
    return textResult(out);
}

json::Value callSet(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);

    const std::string key = args.str("field");

    // Four header fields belong to the record rather than to a screen. They
    // carry no provenance because they are not claims about the project, they
    // are how it is filed.
    if (key == "repo" || key == "summary" || key == "phase" || key == "name") {
        const std::string value = args.str("value");
        if (key == "repo") {
            if (!value.empty() && !pm::Git::isRepository(value))
                return textResult(value + " is not a git work tree.", true);
            record->repo = value;
        } else if (key == "summary") record->summary = value;
        else if (key == "phase") record->phase = value;
        else record->name = value;
        if (!saveProject(record, error)) return textResult(error, true);
        return textResult("Set " + key + " to \"" + value + "\".\n");
    }

    const pm::Field* field = g_workspace.definitions().fieldByKey(key);
    if (!field)
        return textResult("no field \"" + key +
                              "\". Use autopm_screens for the screen fields; the record header "
                              "also takes repo, summary, phase and name.",
                          true);
    if (field->isTable())
        return textResult("\"" + key + "\" is a table. Use autopm_add_row.", true);

    const std::string provenance = args.str("provenance", "untagged");
    const std::string evidence = args.str("evidence");
    if (!provenanceIsUsable(provenance, evidence, error)) return textResult(error, true);

    pm::Entry& entry = record->mutableEntry(key);
    entry.value = args.str("value");
    entry.provenance = pm::provenanceFromString(provenance);
    entry.evidence = evidence;
    if (!saveProject(record, error)) return textResult(error, true);

    std::string out = "Wrote " + key + " [" + provenance + "].\n";
    const std::vector<std::string> notes = pm::reviewField(*field, &entry);
    for (const std::string& note : notes) out += "Coach: " + note + "\n";
    return textResult(out);
}

pm::Row* findRow(std::vector<pm::Row>& rows, const std::string& wanted) {
    for (pm::Row& row : rows)
        if (row.id == wanted || row.cell("ref") == wanted) return &row;
    return nullptr;
}

json::Value callAddRow(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);

    const std::string key = args.str("field");
    const pm::Field* field = g_workspace.definitions().fieldByKey(key);
    if (!field) return textResult("no field \"" + key + "\". Use autopm_screens.", true);
    if (!field->isTable()) return textResult("\"" + key + "\" is not a table. Use autopm_set.", true);

    const std::string provenance = args.str("provenance", "untagged");
    const std::string evidence = args.str("evidence");
    if (!provenanceIsUsable(provenance, evidence, error)) return textResult(error, true);

    // Follow whatever id convention the table already uses, so a record that
    // has been hand-written stays internally consistent.
    std::string prefix = "r";
    if (const std::vector<pm::Row>* existing = record->rows(key)) {
        for (const pm::Row& row : *existing) {
            size_t letters = 0;
            while (letters < row.id.size() && !std::isdigit(static_cast<unsigned char>(row.id[letters])))
                ++letters;
            if (letters > 0 && letters < row.id.size()) { prefix = row.id.substr(0, letters); break; }
        }
    }

    pm::Row row;
    row.id = record->nextRowId(key, prefix);
    row.provenance = pm::provenanceFromString(provenance);
    row.evidence = evidence;

    std::string unknown;
    for (const auto& cell : args["cells"].asObject()) {
        if (!field->column(cell.first)) { unknown += " " + cell.first; continue; }
        row.setCell(cell.first, cell.second.isString() ? cell.second.asString()
                                                       : cell.second.dump());
    }
    if (!unknown.empty())
        return textResult("unknown column(s):" + unknown + ". Use autopm_screens to see them.",
                          true);

    record->mutableRows(key).push_back(row);
    if (!saveProject(record, error)) return textResult(error, true);

    std::string out = "Added row " + row.id + " to " + key + " [" + provenance + "].\n";
    for (const std::string& note : pm::reviewField(*field, record->entry(key)))
        out += "Coach: " + note + "\n";
    return textResult(out);
}

json::Value callUpdateRow(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);

    const std::string key = args.str("field");
    const pm::Field* field = g_workspace.definitions().fieldByKey(key);
    if (!field) return textResult("no field \"" + key + "\".", true);

    const std::string provenance = args.str("provenance");
    const std::string evidence = args.str("evidence");
    if (!provenance.empty() && !provenanceIsUsable(provenance, evidence, error))
        return textResult(error, true);

    std::vector<pm::Row>& rows = record->mutableRows(key);
    pm::Row* row = findRow(rows, args.str("row"));
    if (!row) return textResult("no row \"" + args.str("row") + "\" in " + key + ".", true);

    std::string unknown;
    std::string changed;
    for (const auto& cell : args["cells"].asObject()) {
        if (!field->column(cell.first)) { unknown += " " + cell.first; continue; }
        const std::string value =
            cell.second.isString() ? cell.second.asString() : cell.second.dump();
        changed += " " + cell.first + "=" + value + ";";
        if (value.empty()) row->cells.erase(cell.first);
        else row->setCell(cell.first, value);
    }
    if (!unknown.empty()) return textResult("unknown column(s):" + unknown, true);
    if (!provenance.empty()) {
        row->provenance = pm::provenanceFromString(provenance);
        row->evidence = evidence;
    }
    if (!saveProject(record, error)) return textResult(error, true);
    return textResult("Updated " + key + " row " + row->id + ":" + changed + "\n");
}

json::Value callReview(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);

    const std::string wanted = args.str("screen");
    std::string out = "Review of " + record->name + "\n\n";
    int blank = 0, thin = 0;

    for (const pm::Screen& screen : g_workspace.definitions().screens()) {
        if (!wanted.empty() && screen.id != wanted) continue;
        std::string section;
        for (const pm::Field& field : screen.fields) {
            const std::string key = screen.id + "." + field.id;
            const pm::Entry* entry = record->entry(key);
            if (!entry || entry->isEmpty()) {
                section += "  " + key + "  BLANK — " + field.ifBlank + "\n";
                ++blank;
                continue;
            }
            for (const std::string& note : pm::reviewField(field, entry)) {
                section += "  " + key + "  " + note + "\n";
                ++thin;
            }
        }
        if (!section.empty()) out += screen.title + "\n" + section + "\n";
    }
    if (blank == 0 && thin == 0) out += "Nothing blank, nothing thin.\n";
    else out += std::to_string(blank) + " blank, " + std::to_string(thin) + " thin.\n";
    return textResult(out);
}

std::string repoOf(const std::shared_ptr<pm::Record>& record, std::string& error) {
    if (record->repo.empty()) {
        error = record->name + " does not name a repository. Set one with autopm_set, field "
                               "\"repo\", value the path to the work tree.";
        return {};
    }
    if (!pm::Git::isRepository(record->repo)) {
        error = record->repo + " is not a git work tree.";
        return {};
    }
    return record->repo;
}

json::Value callGitActivity(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);
    const std::string repo = repoOf(record, error);
    if (repo.empty()) return textResult(error, true);

    const int limit = args["limit"].asInt(60);
    const std::vector<pm::Commit> commits =
        pm::Git::log(repo, args.str("since"), args.str("until"), limit);
    if (commits.empty()) return textResult("No commits in that window.\n");

    std::string out = std::to_string(commits.size()) + " commits in " + repo + "\n\n";
    for (const pm::Commit& commit : commits)
        out += commit.date + "  " + commit.hash + "  " + commit.subject + "  (" + commit.author +
               ")\n";
    out += "\nA record entry pinned to one of these hashes is checkable by anyone.\n";
    return textResult(out);
}

json::Value callReconcile(const json::Value& args) {
    std::string error;
    auto record = openProject(args.str("project"), error);
    if (!record) return textResult(error, true);
    const std::string repo = repoOf(record, error);
    if (repo.empty()) return textResult(error, true);

    const std::vector<pm::Row>* sprints = record->rows("sprints.sprints");
    const std::vector<pm::Row>* cards = record->rows("board.cards");
    if (!sprints || sprints->empty())
        return textResult("No sprints recorded, so there is nothing to hold the history up "
                          "against. Add some on the Sprint Planning screen.\n");

    std::string out = "Record against history, " + repo + "\n\n";
    for (const pm::Row& sprint : *sprints) {
        const std::string name = sprint.cell("name");
        const std::string start = sprint.cell("start");
        const std::string end = sprint.cell("end");
        if (name.empty()) continue;

        int claimed = 0;
        int claimedCards = 0;
        if (cards) {
            for (const pm::Row& card : *cards) {
                if (card.cell("sprint") != name || card.cell("state") != "Done") continue;
                ++claimedCards;
                try {
                    claimed += std::stoi(card.cell("points"));
                } catch (...) {
                }
            }
        }
        const std::vector<pm::Commit> commits = pm::Git::log(repo, start, end, 500);

        out += name + "  " + (start.empty() ? "?" : start) + " to " +
               (end.empty() ? "?" : end) + "\n";
        out += "  record: " + std::to_string(claimedCards) + " cards done, " +
               std::to_string(claimed) + " points\n";
        out += "  git:    " + std::to_string(commits.size()) + " commits in that window\n";

        if (claimedCards > 0 && commits.empty())
            out += "  ** claims finished work with no commits behind it in that window. Either "
                   "the dates are wrong or the cards are.\n";
        if (claimedCards == 0 && commits.size() > 3)
            out += "  ** " + std::to_string(commits.size()) +
                   " commits and nothing marked done. The board is behind the work.\n";
        out += "\n";
    }
    out += "This compares claims with commits and nothing more. What it means is yours to "
           "decide; nothing has been changed.\n";
    return textResult(out);
}

json::Value dispatch(const std::string& name, const json::Value& args) {
    if (name == "autopm_projects")     return callProjects();
    if (name == "autopm_create")       return callCreate(args);
    if (name == "autopm_screens")      return callScreens(args);
    if (name == "autopm_read")         return callRead(args);
    if (name == "autopm_set")          return callSet(args);
    if (name == "autopm_add_row")      return callAddRow(args);
    if (name == "autopm_update_row")   return callUpdateRow(args);
    if (name == "autopm_review")       return callReview(args);
    if (name == "autopm_git_activity") return callGitActivity(args);
    if (name == "autopm_reconcile")    return callReconcile(args);
    return textResult("unknown tool \"" + name + "\"", true);
}

// ── transport ────────────────────────────────────────────────────────────

void reply(const json::Value& id, const json::Value& result) {
    json::Object message;
    message["jsonrpc"] = json::Value("2.0");
    message["id"] = id;
    message["result"] = result;
    std::cout << json::Value(message).dump() << "\n" << std::flush;
}

void replyError(const json::Value& id, int code, const std::string& text) {
    json::Object error;
    error["code"] = json::Value(code);
    error["message"] = json::Value(text);
    json::Object message;
    message["jsonrpc"] = json::Value("2.0");
    message["id"] = id;
    message["error"] = json::Value(error);
    std::cout << json::Value(message).dump() << "\n" << std::flush;
}

} // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    // Binary mode, or the newline that delimits messages gets a carriage
    // return added to it and the client's framing breaks.
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stdin), _O_BINARY);
#endif

    std::string definitionsDir;
    g_recordsDir = (std::filesystem::path(documentsDir()) / "AutoPM").string();
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--records" && i + 1 < argc) g_recordsDir = argv[++i];
        else if (arg == "--definitions" && i + 1 < argc) definitionsDir = argv[++i];
    }
    if (definitionsDir.empty()) {
        for (const std::string& candidate : pm::definitionSearchPaths(executableDir())) {
            std::error_code code;
            if (std::filesystem::is_directory(candidate, code)) {
                definitionsDir = std::filesystem::weakly_canonical(candidate, code).string();
                break;
            }
        }
    }

    std::string error;
    const bool ready = g_workspace.open(definitionsDir, g_recordsDir, error);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        json::Value message;
        std::string parseError;
        if (!json::parse(line, message, parseError)) continue;

        const std::string method = message.str("method");
        const json::Value id = message["id"];
        const bool isNotification = !message.has("id");

        if (method == "initialize") {
            json::Object capabilities;
            capabilities["tools"] = json::Value(json::Object{});
            json::Object info;
            info["name"] = json::Value("autopm");
            info["version"] = json::Value("0.1.0");
            json::Object result;
            // Answer in the version the client asked for when it named one.
            const std::string asked = message["params"].str("protocolVersion");
            result["protocolVersion"] = json::Value(asked.empty() ? "2025-06-18" : asked);
            result["capabilities"] = json::Value(capabilities);
            result["serverInfo"] = json::Value(info);
            if (!ready)
                result["instructions"] = json::Value("AutoPM could not load its definitions: " +
                                                     error);
            reply(id, json::Value(result));
            continue;
        }
        if (method == "notifications/initialized" || isNotification) continue;
        if (method == "ping") { reply(id, json::Value(json::Object{})); continue; }
        if (method == "tools/list") { reply(id, listTools()); continue; }
        if (method == "tools/call") {
            if (!ready) {
                reply(id, textResult("AutoPM definitions are not loaded: " + error, true));
                continue;
            }
            const json::Value& params = message["params"];
            reply(id, dispatch(params.str("name"), params["arguments"]));
            continue;
        }
        replyError(id, -32601, "unknown method \"" + method + "\"");
    }
    return 0;
}
