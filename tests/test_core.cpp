// Core tests. No framework: the core is Qt-free and has no dependencies, and
// adding one to assert a few dozen facts would be the largest thing in the
// build.
//
// What is covered is deliberate rather than exhaustive. Every case below is
// either a rule the tool promises (provenance, the block format round trip) or
// a defect that actually happened and could come back: the coach counting
// blank answers as thin, and the walkthrough proposing a screen from a phase
// the project has not reached.
//
//   test_core <definitions-dir> <repo-root>

#include "core/blockfile.h"
#include "core/coach.h"
#include "core/definition.h"
#include "core/gitlog.h"
#include "core/record.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace {

int g_checks = 0;
int g_failures = 0;
const char* g_case = "";

void record(bool ok, const char* expression, int line) {
    ++g_checks;
    if (ok) return;
    ++g_failures;
    std::printf("  FAIL  %s:%d  %s\n", g_case, line, expression);
}

#define CHECK(expr) record((expr), #expr, __LINE__)

void beginCase(const char* name) {
    g_case = name;
    std::printf("%s\n", name);
}

// ── the block format ─────────────────────────────────────────────────────

void testBlockRoundTrip() {
    beginCase("block file: nested nodes survive a write and a reparse");

    const std::string text =
        "screen: charter\n"
        "title: Project Charter\n"
        "order: 10\n"
        "field purpose\n"
        "  label: Purpose\n"
        "  type: text\n"
        "  check words>=10: A purpose in under ten words is a title.\n"
        "  check filled: Say something.\n"
        "field sponsor\n"
        "  label: Sponsor\n"
        "  type: line\n";

    pm::Node root;
    std::string error;
    CHECK(pm::parseBlocks(text, root, error));
    CHECK(error.empty());
    CHECK(root.get("screen") == "charter");
    CHECK(root.getInt("order") == 10);

    const pm::Node* purpose = root.child("field", "purpose");
    CHECK(purpose != nullptr);
    if (purpose) {
        CHECK(purpose->get("type") == "text");
        CHECK(purpose->childrenWithTag("check").size() == 2);
    }
    CHECK(root.childrenWithTag("field").size() == 2);

    // The definition files are edited by hand, so what the writer emits has to
    // parse back to the same thing or a save silently rewrites the study guide.
    pm::Node again;
    CHECK(pm::parseBlocks(pm::writeBlocks(root), again, error));
    CHECK(again.get("screen") == "charter");
    const pm::Node* purposeAgain = again.child("field", "purpose");
    CHECK(purposeAgain != nullptr);
    if (purposeAgain) {
        CHECK(purposeAgain->get("type") == "text");
        CHECK(purposeAgain->childrenWithTag("check").size() == 2);
    }
}

void testBlockScalars() {
    beginCase("block file: multi-line values dedent to the key");

    const std::string text =
        "why: |\n"
        "  first line\n"
        "  second line\n"
        "after: done\n";

    pm::Node root;
    std::string error;
    CHECK(pm::parseBlocks(text, root, error));
    CHECK(root.get("why") == "first line\nsecond line");
    CHECK(root.get("after") == "done");
}

void testRepeatedKeys() {
    beginCase("block file: repeated keys keep their order");

    pm::Node node;
    node.add("prerequisite", "charter");
    node.add("prerequisite", "stakeholders");
    node.set("title", "Scope");
    node.set("title", "Scope Statement");   // set replaces

    const std::vector<std::string> all = node.getAll("prerequisite");
    CHECK(all.size() == 2);
    if (all.size() == 2) {
        CHECK(all[0] == "charter");
        CHECK(all[1] == "stakeholders");
    }
    CHECK(node.getAll("title").size() == 1);
    CHECK(node.get("title") == "Scope Statement");
}

// ── provenance ───────────────────────────────────────────────────────────

void testProvenance() {
    beginCase("provenance: four states, two of which are decisions");

    CHECK(pm::provenanceFromString("specified") == pm::Provenance::Specified);
    CHECK(pm::provenanceFromString("agreed") == pm::Provenance::Agreed);
    CHECK(pm::provenanceFromString("unobjected") == pm::Provenance::Unobjected);
    CHECK(pm::provenanceFromString("") == pm::Provenance::Untagged);
    CHECK(pm::provenanceFromString("nonsense") == pm::Provenance::Untagged);

    CHECK(pm::provenanceToString(pm::Provenance::Specified) == "specified");
    CHECK(pm::provenanceToString(pm::Provenance::Agreed) == "agreed");

    // The whole point of the four states: silence is not agreement, so an
    // unobjected entry is not a decision and the MCP will not let one be
    // recorded as though it were.
    CHECK(pm::provenanceIsDecision(pm::Provenance::Specified));
    CHECK(pm::provenanceIsDecision(pm::Provenance::Agreed));
    CHECK(!pm::provenanceIsDecision(pm::Provenance::Unobjected));
    CHECK(!pm::provenanceIsDecision(pm::Provenance::Untagged));
}

// ── records ──────────────────────────────────────────────────────────────

void testRecordRoundTrip() {
    beginCase("record: values, rows, provenance and evidence survive a save");

    const std::string file =
        (std::filesystem::temp_directory_path() / "autopm-test-record.pmproj").string();

    pm::Record written;
    written.id = "test";
    written.name = "Test Project";
    written.accent = "#B05CF6";
    written.created = "2026-09-15";
    written.phase = "Initiating";
    written.repo = "C:/somewhere/else";

    // A colon inside a value is the obvious hazard in a "key: value" format,
    // and charter prose is full of them.
    written.setValue("charter.purpose", "Build a thing: one that teaches the job.");
    pm::Entry& purpose = written.mutableEntry("charter.purpose");
    purpose.provenance = pm::Provenance::Agreed;
    purpose.evidence = "said so in the session of 2026-09-15";

    pm::Row risk;
    risk.id = "R1";
    risk.setCell("risk", "The tool gets learned instead of the job");
    risk.setCell("owner", "the sponsor");
    risk.provenance = pm::Provenance::Specified;
    written.mutableRows("risks.register").push_back(risk);

    std::string error;
    CHECK(written.save(file, error));

    pm::Record read;
    CHECK(read.load(file, error));
    CHECK(read.id == "test");
    CHECK(read.name == "Test Project");
    CHECK(read.accent == "#B05CF6");
    CHECK(read.phase == "Initiating");
    CHECK(read.repo == "C:/somewhere/else");
    CHECK(read.value("charter.purpose") == "Build a thing: one that teaches the job.");

    const pm::Entry* entry = read.entry("charter.purpose");
    CHECK(entry != nullptr);
    if (entry) {
        CHECK(entry->provenance == pm::Provenance::Agreed);
        CHECK(entry->evidence == "said so in the session of 2026-09-15");
    }

    const std::vector<pm::Row>* rows = read.rows("risks.register");
    CHECK(rows != nullptr);
    if (rows && rows->size() == 1) {
        CHECK((*rows)[0].id == "R1");
        CHECK((*rows)[0].cell("risk") == "The tool gets learned instead of the job");
        CHECK((*rows)[0].cell("owner") == "the sponsor");
        CHECK((*rows)[0].provenance == pm::Provenance::Specified);
    } else {
        CHECK(rows != nullptr && rows->size() == 1);
    }

    std::error_code ignored;
    std::filesystem::remove(file, ignored);
}

// ── definitions ──────────────────────────────────────────────────────────

void testDefinitionsLoad(const std::string& definitionsDir) {
    beginCase("definitions: the shipped screens parse");

    pm::Definitions definitions;
    std::string error;
    CHECK(definitions.loadDirectory(definitionsDir, error));
    CHECK(error.empty());
    if (!error.empty()) std::printf("  error: %s\n", error.c_str());

    CHECK(definitions.screens().size() >= 13);
    CHECK(definitions.screen("charter") != nullptr);
    CHECK(definitions.screen("board") != nullptr);

    // A view that works on another screen's rows names it as "screen.field".
    // A typo there is invisible until the board opens empty.
    CHECK(definitions.fieldByKey("board.cards") != nullptr);

    for (const pm::Screen& screen : definitions.screens()) {
        CHECK(!screen.id.empty());
        CHECK(!screen.title.empty());
        CHECK(!screen.phase.empty());
        CHECK(!screen.fields.empty());
        for (const pm::Field& field : screen.fields) {
            CHECK(!field.id.empty());
            CHECK(!field.label.empty());
            // Every "reads" has to resolve, or the view it drives is blank.
            if (!field.reads.empty()) CHECK(definitions.fieldByKey(field.reads) != nullptr);
        }
    }

    // Lifecycle order, derived from screen order. Intake sits before the
    // charter, because the business case is written before a project exists.
    const std::vector<std::string> phases = definitions.phases();
    CHECK(!phases.empty());
    const auto at = [&phases](const std::string& name) {
        return std::find(phases.begin(), phases.end(), name) - phases.begin();
    };
    CHECK(phases.front() == "Before the project");
    CHECK(at("Before the project") < at("Initiating"));
    CHECK(at("Initiating") < at("Planning"));
    CHECK(at("Planning") < at("Executing"));
    CHECK(at("Executing") < at("Closing"));
}

// ── the coach ────────────────────────────────────────────────────────────

void testBlankIsNotThin(const std::string& definitionsDir) {
    beginCase("coach: an unanswered field is not a thin answer");

    pm::Definitions definitions;
    std::string error;
    if (!definitions.loadDirectory(definitionsDir, error)) {
        CHECK(false);
        return;
    }
    const pm::Screen* charter = definitions.screen("charter");
    CHECK(charter != nullptr);
    if (!charter) return;

    pm::Record empty;
    empty.id = "blank";
    empty.phase = "Initiating";

    // Nothing answered, so there is nothing to criticise yet. Reporting every
    // untouched field as thin is what made a fresh project claim 28 problems.
    pm::Progress fresh = pm::progressOf(*charter, empty);
    CHECK(fresh.fields > 0);
    CHECK(fresh.answered == 0);
    CHECK(fresh.notes == 0);

    // An entry that exists but holds nothing is still unanswered.
    empty.setValue("charter.purpose", "");
    pm::Progress blank = pm::progressOf(*charter, empty);
    CHECK(blank.answered == 0);
    CHECK(blank.notes == 0);

    // But a real answer that fails a rule does get a note, or the check above
    // would pass on a coach that never says anything.
    pm::Record thin;
    thin.id = "thin";
    thin.phase = "Initiating";
    thin.setValue("charter.purpose", "A tool.");
    pm::Progress counted = pm::progressOf(*charter, thin);
    CHECK(counted.answered == 1);
    CHECK(counted.notes >= 1);

    const pm::Field* purpose = charter->field("purpose");
    CHECK(purpose != nullptr);
    if (purpose) {
        CHECK(pm::reviewField(*purpose, thin.entry("charter.purpose")).size() >= 1);
        // reviewField is the raw rule check, and a blank answer really does
        // fail "ten words". Deciding that a blank is not worth saying out loud
        // belongs to whoever summarises a whole screen, below.
        CHECK(!pm::reviewField(*purpose, nullptr).empty());
    }

    // A screen nobody has started has nothing to criticise, and the note list
    // and the answered count have to agree on what a note is.
    CHECK(pm::reviewScreen(*charter, empty).empty());
    CHECK(pm::reviewScreen(*charter, thin).size() == static_cast<size_t>(counted.notes));
}

void testNextScreenStaysInPhase(const std::string& definitionsDir) {
    beginCase("coach: the next screen is never one from a later phase");

    pm::Definitions definitions;
    std::string error;
    if (!definitions.loadDirectory(definitionsDir, error)) {
        CHECK(false);
        return;
    }

    // A project that has only just been authorised should be sent to the
    // charter, not to closeout, which is empty for the correct reason.
    pm::Record starting;
    starting.id = "starting";
    starting.phase = "Initiating";

    const std::vector<std::string> phases = definitions.phases();
    const auto at = [&phases](const std::string& name) {
        return std::find(phases.begin(), phases.end(), name) - phases.begin();
    };

    const std::string next = pm::nextScreen(definitions, starting);
    CHECK(!next.empty());
    const pm::Screen* proposed = definitions.screen(next);
    CHECK(proposed != nullptr);
    // Never a phase the project has not reached. An earlier one is fair: the
    // business case belongs before the charter and is exactly what a project
    // that has only just been authorised is likely to be missing.
    if (proposed) CHECK(at(proposed->phase) <= at(starting.phase));

    // Further along, later screens become reachable.
    pm::Record executing;
    executing.id = "executing";
    executing.phase = "Executing";
    const pm::Screen* later = definitions.screen(pm::nextScreen(definitions, executing));
    CHECK(later != nullptr);
    if (later) CHECK(later->phase != "Closing");
}

void testCoversCheck() {
    beginCase("coach: covers: catches a criterion the closeout never answered");

    pm::Check parsed;
    CHECK(pm::parseCheck("covers:charter.success_criteria", "one is missing", parsed));
    CHECK(parsed.kind == pm::Check::Kind::Covers);
    CHECK(parsed.argument == "charter.success_criteria");

    pm::Record project;
    project.id = "covers";
    for (const char* ref : {"S1", "S2", "S3"}) {
        pm::Row row;
        row.id = ref;
        row.setCell("ref", ref);
        row.setCell("criterion", "something checkable");
        project.mutableRows("charter.success_criteria").push_back(row);
    }

    pm::Field outcome;
    outcome.id = "outcome";
    outcome.label = "Outcome";
    outcome.type = "table";
    outcome.checks.push_back(parsed);

    // Two of three answered. The one left out is the one that went badly, and
    // before this check that closeout passed the coach in silence.
    for (const char* ref : {"S1", "S2"}) {
        pm::Row row;
        row.id = std::string("A") + ref;
        row.setCell("traces", ref);
        project.mutableRows("closeout.outcome").push_back(row);
    }
    CHECK(pm::reviewField(outcome, project.entry("closeout.outcome"), &project).size() == 1);

    pm::Row third;
    third.id = "AS3";
    third.setCell("traces", "S3");
    project.mutableRows("closeout.outcome").push_back(third);
    CHECK(pm::reviewField(outcome, project.entry("closeout.outcome"), &project).empty());

    // With no record to compare against the check cannot see what it covers,
    // and says nothing rather than failing an answer it cannot check.
    CHECK(pm::reviewField(outcome, project.entry("closeout.outcome")).empty());
}

void testSeedRecord(const std::string& definitionsDir, const std::string& repoRoot) {
    beginCase("seed: the demo record loads and answers its own charter");

    const std::string file = repoRoot + "/assets/seed/demo.pmproj";
    pm::Record seed;
    std::string error;
    CHECK(seed.load(file, error));
    if (!error.empty()) {
        std::printf("  error: %s\n", error.c_str());
        return;
    }
    CHECK(!seed.name.empty());

    // Success criteria are rows, not a paragraph. A record left half-migrated
    // keeps its old value: alongside the new rows and still reads as answered,
    // which is the one way this change breaks quietly.
    const pm::Entry* criteria = seed.entry("charter.success_criteria");
    CHECK(criteria != nullptr);
    if (criteria) {
        CHECK(criteria->rows.size() >= 3);
        CHECK(criteria->value.empty());
    }

    // Every criterion is answered at closeout, matched on what it traces to.
    pm::Definitions definitions;
    if (!definitions.loadDirectory(definitionsDir, error)) {
        CHECK(false);
        return;
    }
    const pm::Screen* closeout = definitions.screen("closeout");
    CHECK(closeout != nullptr);
    const pm::Field* outcome = closeout ? closeout->field("outcome") : nullptr;
    CHECK(outcome != nullptr);
    if (outcome) {
        // Only the covers: check. The rest of the field's checks fire here for
        // the right reason: this project has not closed, so nobody has said
        // whether any criterion was met, and an unanswered met column on a
        // running project is the correct state rather than a defect.
        pm::Field coversOnly = *outcome;
        coversOnly.checks.erase(
            std::remove_if(coversOnly.checks.begin(), coversOnly.checks.end(),
                           [](const pm::Check& check) {
                               return check.kind != pm::Check::Kind::Covers;
                           }),
            coversOnly.checks.end());
        CHECK(coversOnly.checks.size() == 1);
        for (const std::string& note :
             pm::reviewField(coversOnly, seed.entry("closeout.outcome"), &seed))
            std::printf("  note: %s\n", note.c_str());
        CHECK(pm::reviewField(coversOnly, seed.entry("closeout.outcome"), &seed).empty());
    }
}

// ── git ──────────────────────────────────────────────────────────────────

void testGitRefusesHostileArguments(const std::string& repoRoot) {
    beginCase("git: an argument that could reach the shell is refused");

    // The command is built as a string and handed to the shell, so every
    // argument is screened first. These must be refused whatever is on disk.
    CHECK(!pm::Git::isRepository("C:/tmp/\"; echo pwned; #"));
    CHECK(!pm::Git::isRepository("C:/tmp/`whoami`"));
    CHECK(!pm::Git::isRepository("C:/tmp/$(whoami)"));
    CHECK(!pm::Git::isRepository("C:/tmp/%USERPROFILE%"));
    CHECK(!pm::Git::isRepository("C:/tmp/a\nb"));

    // The same directory, with and without a hostile date. Only the argument
    // differs, so this distinguishes screening from "there was nothing there".
    if (pm::Git::isRepository(repoRoot)) {
        CHECK(!pm::Git::log(repoRoot, "", "", 5).empty());
        CHECK(pm::Git::log(repoRoot, "$(whoami)", "", 5).empty());
        CHECK(pm::Git::log(repoRoot, "", "`whoami`", 5).empty());
    } else {
        std::printf("  (skipped the repository half: %s is not a work tree)\n",
                    repoRoot.c_str());
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::printf("usage: test_core <definitions-dir> <repo-root>\n");
        return 2;
    }
    const std::string definitionsDir = argv[1];
    const std::string repoRoot = argv[2];

    testBlockRoundTrip();
    testBlockScalars();
    testRepeatedKeys();
    testProvenance();
    testRecordRoundTrip();
    testDefinitionsLoad(definitionsDir);
    testBlankIsNotThin(definitionsDir);
    testNextScreenStaysInPhase(definitionsDir);
    testCoversCheck();
    testSeedRecord(definitionsDir, repoRoot);
    testGitRefusesHostileArguments(repoRoot);

    std::printf("\n%d checks, %d failed\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
