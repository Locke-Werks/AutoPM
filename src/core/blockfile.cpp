#include "blockfile.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace pm {
namespace {

std::string rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(" \t\r\n");
    return end == std::string::npos ? std::string() : s.substr(0, end + 1);
}

std::string ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    return start == std::string::npos ? std::string() : s.substr(start);
}

std::string trim(const std::string& s) { return ltrim(rtrim(s)); }

size_t indentOf(const std::string& line) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ') ++i;
    return i;
}

bool isBlank(const std::string& line) { return trim(line).empty(); }

bool isComment(const std::string& line) {
    std::string t = ltrim(line);
    return !t.empty() && t[0] == '#';
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;
    for (char c : text) {
        if (c == '\n') {
            lines.push_back(rtrim(current));
            current.clear();
        } else if (c != '\r') {
            current += c;
        }
    }
    if (!current.empty()) lines.push_back(rtrim(current));
    return lines;
}

// A line is a key/value when its first token ends with a colon. Anything else
// that carries content opens a child node, so "field purpose" nests while
// "label: Project title" does not.
bool splitKeyValue(const std::string& body, std::string& key, std::string& value) {
    size_t colon = body.find(':');
    if (colon == std::string::npos) return false;
    size_t space = body.find(' ');
    if (space != std::string::npos && space < colon) return false;
    key = trim(body.substr(0, colon));
    value = trim(body.substr(colon + 1));
    return !key.empty();
}

struct Parser {
    const std::vector<std::string>& lines;
    size_t pos = 0;
    std::string error;

    explicit Parser(const std::vector<std::string>& l) : lines(l) {}

    bool atEnd() const { return pos >= lines.size(); }

    // Consumes every line indented deeper than the given indent into node.
    bool parseInto(Node& node, size_t indent) {
        while (!atEnd()) {
            const std::string& line = lines[pos];
            if (isBlank(line) || isComment(line)) { ++pos; continue; }

            size_t lineIndent = indentOf(line);
            if (lineIndent < indent) return true;
            if (lineIndent > indent) {
                error = "unexpected indent at line " + std::to_string(pos + 1) + ": " + trim(line);
                return false;
            }

            std::string body = trim(line);
            ++pos;

            std::string key, value;
            if (splitKeyValue(body, key, value)) {
                if (value == "|") {
                    node.attrs.emplace_back(key, readBlockScalar(indent));
                } else {
                    node.attrs.emplace_back(key, value);
                }
                continue;
            }

            Node child;
            size_t space = body.find(' ');
            if (space == std::string::npos) {
                child.tag = body;
            } else {
                child.tag = body.substr(0, space);
                child.id = trim(body.substr(space + 1));
            }
            if (!parseInto(child, indent + 2)) return false;
            node.children.push_back(std::move(child));
        }
        return true;
    }

    // Everything indented past the key belongs to the block, blank lines
    // included, so an explanation can carry paragraph breaks.
    std::string readBlockScalar(size_t keyIndent) {
        size_t blockIndent = keyIndent + 2;
        std::vector<std::string> collected;
        while (!atEnd()) {
            const std::string& line = lines[pos];
            if (isBlank(line)) {
                collected.push_back(std::string());
                ++pos;
                continue;
            }
            if (indentOf(line) < blockIndent) break;
            collected.push_back(line.substr(std::min(blockIndent, line.size())));
            ++pos;
        }
        while (!collected.empty() && collected.back().empty()) collected.pop_back();
        std::string out;
        for (size_t i = 0; i < collected.size(); ++i) {
            if (i) out += "\n";
            out += collected[i];
        }
        return out;
    }
};

void writeNode(std::ostringstream& out, const Node& node, size_t indent) {
    std::string pad(indent, ' ');
    for (const auto& attr : node.attrs) {
        if (attr.second.find('\n') != std::string::npos) {
            out << pad << attr.first << ": |\n";
            std::string block(indent + 2, ' ');
            std::istringstream stream(attr.second);
            std::string line;
            while (std::getline(stream, line)) {
                if (line.empty()) out << "\n";
                else out << block << line << "\n";
            }
        } else {
            out << pad << attr.first << ": " << attr.second << "\n";
        }
    }
    for (const auto& child : node.children) {
        out << pad << child.tag;
        if (!child.id.empty()) out << " " << child.id;
        out << "\n";
        writeNode(out, child, indent + 2);
    }
}

} // namespace

bool Node::has(const std::string& key) const {
    for (const auto& attr : attrs)
        if (attr.first == key) return true;
    return false;
}

std::string Node::get(const std::string& key, const std::string& fallback) const {
    for (const auto& attr : attrs)
        if (attr.first == key) return attr.second;
    return fallback;
}

int Node::getInt(const std::string& key, int fallback) const {
    if (!has(key)) return fallback;
    try {
        return std::stoi(get(key));
    } catch (...) {
        return fallback;
    }
}

void Node::set(const std::string& key, const std::string& value) {
    for (auto& attr : attrs) {
        if (attr.first == key) { attr.second = value; return; }
    }
    attrs.emplace_back(key, value);
}

void Node::add(const std::string& key, const std::string& value) {
    attrs.emplace_back(key, value);
}

std::vector<const Node*> Node::childrenWithTag(const std::string& wanted) const {
    std::vector<const Node*> found;
    for (const auto& child : children)
        if (child.tag == wanted) found.push_back(&child);
    return found;
}

const Node* Node::child(const std::string& wanted, const std::string& wantedId) const {
    for (const auto& child : children)
        if (child.tag == wanted && child.id == wantedId) return &child;
    return nullptr;
}

bool parseBlocks(const std::string& text, Node& root, std::string& error) {
    std::vector<std::string> lines = splitLines(text);
    Parser parser(lines);
    root = Node();
    if (!parser.parseInto(root, 0)) {
        error = parser.error;
        return false;
    }
    return true;
}

std::string writeBlocks(const Node& root) {
    std::ostringstream out;
    writeNode(out, root, 0);
    return out.str();
}

bool readFile(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::ostringstream buffer;
    buffer << in.rdbuf();
    out = buffer.str();
    return true;
}

// Write to a sibling temp file and rename, so a crash mid-save cannot leave a
// half-written record behind.
bool writeFileAtomic(const std::string& path, const std::string& text) {
    std::string temp = path + ".tmp";
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << text;
        if (!out) return false;
    }
    std::remove(path.c_str());
    return std::rename(temp.c_str(), path.c_str()) == 0;
}

} // namespace pm
