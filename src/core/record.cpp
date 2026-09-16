#include "record.h"

#include "blockfile.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <sstream>

namespace pm {

std::string provenanceToString(Provenance p) {
    switch (p) {
        case Provenance::Specified:  return "specified";
        case Provenance::Agreed:     return "agreed";
        case Provenance::Unobjected: return "unobjected";
        case Provenance::Untagged:   break;
    }
    return "untagged";
}

Provenance provenanceFromString(const std::string& s) {
    if (s == "specified")  return Provenance::Specified;
    if (s == "agreed")     return Provenance::Agreed;
    if (s == "unobjected") return Provenance::Unobjected;
    return Provenance::Untagged;
}

bool provenanceIsDecision(Provenance p) {
    return p == Provenance::Specified || p == Provenance::Agreed;
}

std::string Row::cell(const std::string& column) const {
    auto it = cells.find(column);
    return it == cells.end() ? std::string() : it->second;
}

void Row::setCell(const std::string& column, const std::string& value) {
    cells[column] = value;
}

bool Record::hasEntry(const std::string& key) const {
    return entries_.find(key) != entries_.end();
}

const Entry* Record::entry(const std::string& key) const {
    auto it = entries_.find(key);
    return it == entries_.end() ? nullptr : &it->second;
}

Entry& Record::mutableEntry(const std::string& key) {
    return entries_[key];
}

void Record::removeEntry(const std::string& key) {
    entries_.erase(key);
}

std::string Record::value(const std::string& key) const {
    const Entry* e = entry(key);
    return e ? e->value : std::string();
}

void Record::setValue(const std::string& key, const std::string& text) {
    entries_[key].value = text;
}

const std::vector<Row>* Record::rows(const std::string& key) const {
    const Entry* e = entry(key);
    return e ? &e->rows : nullptr;
}

std::vector<Row>& Record::mutableRows(const std::string& key) {
    return entries_[key].rows;
}

std::string Record::nextRowId(const std::string& key, const std::string& prefix) const {
    const std::vector<Row>* existing = rows(key);
    int highest = 0;
    if (existing) {
        for (const Row& row : *existing) {
            if (row.id.size() <= prefix.size() || row.id.compare(0, prefix.size(), prefix) != 0)
                continue;
            try {
                highest = std::max(highest, std::stoi(row.id.substr(prefix.size())));
            } catch (...) {
                // A hand-edited id that is not prefix+number simply does not
                // take part in the numbering.
            }
        }
    }
    return prefix + std::to_string(highest + 1);
}

namespace {

long long stampOf(const std::string& file) {
    std::error_code code;
    const auto when = std::filesystem::last_write_time(file, code);
    if (code) return 0;
    return static_cast<long long>(when.time_since_epoch().count());
}

} // namespace

bool Record::changedOnDisk() const {
    if (path.empty() || stamp_ == 0) return false;
    const long long now = stampOf(path);
    return now != 0 && now != stamp_;
}

bool Record::reload(std::string& error) {
    return load(path, error);
}

bool Record::load(const std::string& file, std::string& error) {
    std::string text;
    if (!readFile(file, text)) {
        error = "could not read " + file;
        return false;
    }
    Node root;
    if (!parseBlocks(text, root, error)) return false;

    entries_.clear();
    id = root.get("project");
    name = root.get("name", id);
    accent = root.get("accent");
    created = root.get("created");
    modified = root.get("modified");
    phase = root.get("phase");
    summary = root.get("summary");
    repo = root.get("repo");
    path = file;

    for (const Node* node : root.childrenWithTag("field")) {
        Entry entry;
        entry.provenance = provenanceFromString(node->get("provenance", "untagged"));
        entry.evidence = node->get("evidence");
        entry.value = node->get("value");
        for (const Node* rowNode : node->childrenWithTag("row")) {
            Row row;
            row.id = rowNode->id;
            row.provenance = provenanceFromString(rowNode->get("provenance", "untagged"));
            row.evidence = rowNode->get("evidence");
            for (const Node* cellNode : rowNode->childrenWithTag("cell"))
                row.cells[cellNode->id] = cellNode->get("value");
            // A cell short enough to sit on one line is written inline as
            // "cell: <column> = <value>"; accept that form too.
            for (const auto& attr : rowNode->attrs) {
                if (attr.first != "cell") continue;
                size_t equals = attr.second.find('=');
                if (equals == std::string::npos) continue;
                std::string column = attr.second.substr(0, equals);
                std::string value = attr.second.substr(equals + 1);
                while (!column.empty() && column.back() == ' ') column.pop_back();
                while (!value.empty() && value.front() == ' ') value.erase(value.begin());
                row.cells[column] = value;
            }
            entry.rows.push_back(std::move(row));
        }
        entries_[node->id] = std::move(entry);
    }

    stamp_ = stampOf(file);
    dirty_ = false;
    return true;
}

std::string Record::serialize() const {
    Node root;
    root.add("project", id);
    root.add("name", name);
    if (!accent.empty()) root.add("accent", accent);
    root.add("created", created);
    root.add("modified", modified);
    if (!phase.empty()) root.add("phase", phase);
    if (!summary.empty()) root.add("summary", summary);
    if (!repo.empty()) root.add("repo", repo);

    for (const auto& pair : entries_) {
        const Entry& entry = pair.second;
        if (entry.isEmpty() && entry.provenance == Provenance::Untagged && entry.evidence.empty())
            continue;

        Node node;
        node.tag = "field";
        node.id = pair.first;
        if (entry.provenance != Provenance::Untagged)
            node.add("provenance", provenanceToString(entry.provenance));
        if (!entry.evidence.empty()) node.add("evidence", entry.evidence);
        if (!entry.value.empty()) node.add("value", entry.value);

        for (const Row& row : entry.rows) {
            Node rowNode;
            rowNode.tag = "row";
            rowNode.id = row.id;
            if (row.provenance != Provenance::Untagged)
                rowNode.add("provenance", provenanceToString(row.provenance));
            if (!row.evidence.empty()) rowNode.add("evidence", row.evidence);
            // One line per cell, so changing one cell is a one-line diff.
            for (const auto& cell : row.cells) {
                if (cell.second.empty()) continue;
                Node cellNode;
                cellNode.tag = "cell";
                cellNode.id = cell.first;
                cellNode.add("value", cell.second);
                rowNode.children.push_back(std::move(cellNode));
            }
            node.children.push_back(std::move(rowNode));
        }
        root.children.push_back(std::move(node));
    }

    std::string header =
        "# AutoPM project record. Plain text on purpose: it is meant to be read\n"
        "# and diffed. Keys are screen.field, matching the files in definitions/.\n"
        "# provenance is one of specified, agreed, unobjected. Anything else is\n"
        "# untagged, and only specified and agreed count as decisions.\n\n";
    return header + writeBlocks(root);
}

bool Record::save(const std::string& file, std::string& error) {
    modified = todayIso();
    if (!writeFileAtomic(file, serialize())) {
        error = "could not write " + file;
        return false;
    }
    path = file;
    stamp_ = stampOf(file);
    dirty_ = false;
    return true;
}

std::string todayIso() {
    std::time_t now = std::time(nullptr);
    std::tm parts{};
#ifdef _WIN32
    localtime_s(&parts, &now);
#else
    localtime_r(&now, &parts);
#endif
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
                  parts.tm_year + 1900, parts.tm_mon + 1, parts.tm_mday);
    return buffer;
}

std::string slugify(const std::string& text) {
    std::string out;
    bool lastWasDash = true;
    for (unsigned char c : text) {
        if (std::isalnum(c)) {
            out += static_cast<char>(std::tolower(c));
            lastWasDash = false;
        } else if (!lastWasDash) {
            out += '-';
            lastWasDash = true;
        }
    }
    while (!out.empty() && out.back() == '-') out.pop_back();
    return out.empty() ? std::string("project") : out;
}

} // namespace pm
