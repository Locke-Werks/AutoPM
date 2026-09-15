#include "coach.h"

#include <algorithm>
#include <cctype>

namespace pm {
namespace {

std::string trimCopy(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return std::string();
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> splitCommas(const std::string& text) {
    std::vector<std::string> parts;
    std::string current;
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == ',') {
            const std::string item = trimCopy(current);
            current.clear();
            if (!item.empty()) parts.push_back(item);
        } else {
            current += text[i];
        }
    }
    return parts;
}

int wordCount(const std::string& text) {
    int words = 0;
    bool inWord = false;
    for (unsigned char c : text) {
        if (std::isspace(c)) {
            inWord = false;
        } else if (!inWord) {
            inWord = true;
            ++words;
        }
    }
    return words;
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Any yyyy-mm-dd anywhere in the text.
bool hasIsoDate(const std::string& text) {
    for (size_t i = 0; i + 10 <= text.size(); ++i) {
        const std::string window = text.substr(i, 10);
        bool shaped = true;
        for (int at = 0; at < 10; ++at) {
            const char c = window[static_cast<size_t>(at)];
            if (at == 4 || at == 7) shaped = shaped && c == '-';
            else shaped = shaped && std::isdigit(static_cast<unsigned char>(c)) != 0;
        }
        if (shaped) return true;
    }
    return false;
}

} // namespace

bool parseCheck(const std::string& rule, const std::string& message, Check& out) {
    const std::string cleaned = trimCopy(rule);
    out.message = trimCopy(message);
    if (out.message.empty()) return false;

    if (cleaned == "filled") {
        out.kind = Check::Kind::Filled;
        return true;
    }
    if (cleaned == "date") {
        out.kind = Check::Kind::MentionsDate;
        return true;
    }
    const size_t atLeast = cleaned.find(">=");
    if (atLeast != std::string::npos) {
        const std::string what = trimCopy(cleaned.substr(0, atLeast));
        const std::string number = trimCopy(cleaned.substr(atLeast + 2));
        try {
            out.number = std::stoi(number);
        } catch (...) {
            return false;
        }
        if (what == "words") { out.kind = Check::Kind::MinWords; return true; }
        if (what == "rows")  { out.kind = Check::Kind::MinRows;  return true; }
        return false;
    }
    const size_t colon = cleaned.find(':');
    if (colon != std::string::npos) {
        const std::string what = trimCopy(cleaned.substr(0, colon));
        const std::string argument = trimCopy(cleaned.substr(colon + 1));
        if (what == "column")   { out.kind = Check::Kind::EveryRowHas; out.argument = argument; return true; }
        if (what == "mentions") { out.kind = Check::Kind::MentionsAny; out.argument = argument; return true; }
    }
    return false;
}

std::vector<std::string> reviewField(const Field& field, const Entry* entry) {
    std::vector<std::string> notes;
    const std::string value = entry ? entry->value : std::string();
    const std::vector<Row> rows = entry ? entry->rows : std::vector<Row>();

    for (const Check& check : field.checks) {
        bool passed = true;
        switch (check.kind) {
            case Check::Kind::Filled:
                passed = !value.empty() || !rows.empty();
                break;
            case Check::Kind::MinWords:
                passed = wordCount(value) >= check.number;
                break;
            case Check::Kind::MinRows:
                passed = static_cast<int>(rows.size()) >= check.number;
                break;
            case Check::Kind::EveryRowHas: {
                // An empty table is the MinRows check's business, not this one.
                passed = true;
                for (const Row& row : rows) {
                    if (row.cell(check.argument).empty()) { passed = false; break; }
                }
                break;
            }
            case Check::Kind::MentionsDate:
                passed = hasIsoDate(value);
                if (!passed) {
                    for (const Row& row : rows) {
                        for (const auto& cell : row.cells) {
                            if (hasIsoDate(cell.second)) { passed = true; break; }
                        }
                        if (passed) break;
                    }
                }
                break;
            case Check::Kind::MentionsAny: {
                const std::string haystack = toLower(value);
                passed = false;
                for (const std::string& needle : splitCommas(check.argument)) {
                    if (haystack.find(toLower(needle)) != std::string::npos) { passed = true; break; }
                }
                break;
            }
        }
        if (!passed) notes.push_back(check.message);
    }
    return notes;
}

std::vector<Note> reviewScreen(const Screen& screen, const Record& record) {
    std::vector<Note> notes;
    for (const Field& field : screen.fields) {
        const Entry* entry = record.entry(screen.id + "." + field.id);
        for (const std::string& message : reviewField(field, entry))
            notes.push_back(Note{field.id, message});
    }
    return notes;
}

Progress progressOf(const Screen& screen, const Record& record) {
    Progress progress;
    for (const Field& field : screen.fields) {
        ++progress.fields;
        const Entry* entry = record.entry(screen.id + "." + field.id);
        if (entry && !entry->isEmpty()) ++progress.answered;
        progress.notes += static_cast<int>(reviewField(field, entry).size());
    }
    return progress;
}

std::vector<std::string> unmetPrerequisites(const Screen& screen, const Definitions& definitions,
                                            const Record& record) {
    std::vector<std::string> unmet;
    for (const std::string& id : screen.prerequisites) {
        const Screen* earlier = definitions.screen(id);
        if (!earlier) continue;
        if (!progressOf(*earlier, record).complete()) unmet.push_back(earlier->title);
    }
    return unmet;
}

std::string nextScreen(const Definitions& definitions, const Record& record) {
    for (const Screen& screen : definitions.screens()) {
        if (!progressOf(screen, record).complete()) return screen.id;
    }
    return std::string();
}

} // namespace pm
