// Indentation-based block format. One parser serves both the field definition
// files and the project records, so a person reading either sees the same shape.
//
//   key: value
//   key: |
//     a block of text, dedented to the key's indent + 2
//   node id
//     key: value
//     child id
//       key: value
//
// Chosen over JSON because the definition files are the study guide: Nyx has to
// be able to read them, and a record has to diff one line per changed cell.
#pragma once

#include <string>
#include <vector>

namespace pm {

class Node {
public:
    std::string tag;   // the word that opened the block ("field", "column", "row")
    std::string id;    // the rest of that line
    std::vector<std::pair<std::string, std::string>> attrs;
    std::vector<Node> children;

    bool has(const std::string& key) const;
    std::string get(const std::string& key, const std::string& fallback = std::string()) const;
    std::vector<std::string> getAll(const std::string& key) const;   // repeated keys, in order
    int getInt(const std::string& key, int fallback = 0) const;
    void set(const std::string& key, const std::string& value);   // replaces, or appends
    void add(const std::string& key, const std::string& value);   // always appends

    std::vector<const Node*> childrenWithTag(const std::string& tag) const;
    const Node* child(const std::string& tag, const std::string& id) const;
};

// Returns false and fills `error` when the text is not well formed.
bool parseBlocks(const std::string& text, Node& root, std::string& error);
std::string writeBlocks(const Node& root);

bool readFile(const std::string& path, std::string& out);
bool writeFileAtomic(const std::string& path, const std::string& text);

} // namespace pm
