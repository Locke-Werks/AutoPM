// A project record: one readable file per project.
//
// Every entry carries its provenance, because a decision record is only
// trustworthy if it shows where each entry came from. Four states, nothing
// else: untagged, specified, agreed, unobjected. Only specified and agreed
// count as decisions.
#pragma once

#include <map>
#include <string>
#include <vector>

namespace pm {

enum class Provenance { Untagged, Specified, Agreed, Unobjected };

std::string provenanceToString(Provenance p);
Provenance provenanceFromString(const std::string& s);
bool provenanceIsDecision(Provenance p);

struct Row {
    std::string id;
    std::map<std::string, std::string> cells;
    Provenance provenance = Provenance::Untagged;
    std::string evidence;

    std::string cell(const std::string& column) const;
    void setCell(const std::string& column, const std::string& value);
};

struct Entry {
    std::string value;          // scalar fields
    std::vector<Row> rows;      // table fields
    Provenance provenance = Provenance::Untagged;
    std::string evidence;

    bool isEmpty() const { return value.empty() && rows.empty(); }
};

class Record {
public:
    // Header
    std::string id;
    std::string name;
    std::string accent;         // one accent per project, house rule
    std::string created;
    std::string modified;
    std::string phase;          // the lifecycle phase the project is in now
    std::string summary;
    // The work tree whose history this record documents. Read only, and only
    // for evidence: AutoPM never reports git state, which is ProjectMan's job.
    std::string repo;

    std::string path;           // where it was loaded from; not written into the file

    bool load(const std::string& file, std::string& error);
    bool save(const std::string& file, std::string& error);
    std::string serialize() const;

    // key is "screen.field"
    bool hasEntry(const std::string& key) const;
    const Entry* entry(const std::string& key) const;
    Entry& mutableEntry(const std::string& key);
    void removeEntry(const std::string& key);

    std::string value(const std::string& key) const;
    void setValue(const std::string& key, const std::string& value);

    const std::vector<Row>* rows(const std::string& key) const;
    std::vector<Row>& mutableRows(const std::string& key);

    // Next free row id for a table, as a short stable string ("R7").
    std::string nextRowId(const std::string& key, const std::string& prefix) const;

    const std::map<std::string, Entry>& entries() const { return entries_; }

    bool dirty() const { return dirty_; }
    void markDirty() { dirty_ = true; }
    void markClean() { dirty_ = false; }

private:
    std::map<std::string, Entry> entries_;
    bool dirty_ = false;
};

std::string todayIso();
std::string slugify(const std::string& text);

} // namespace pm
