#include "definition.h"

#include "blockfile.h"

#include <algorithm>
#include <filesystem>

namespace pm {
namespace {

std::string trimCopy(std::string s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return std::string();
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

// "Not started=faint, In progress=accent, Reached=ok" or a plain comma list.
std::vector<Option> parseOptions(const std::string& spec) {
    std::vector<Option> options;
    std::string current;
    for (size_t i = 0; i <= spec.size(); ++i) {
        if (i == spec.size() || spec[i] == ',') {
            std::string item = trimCopy(current);
            current.clear();
            if (item.empty()) continue;
            Option option;
            size_t equals = item.find('=');
            if (equals == std::string::npos) {
                option.value = item;
            } else {
                option.value = trimCopy(item.substr(0, equals));
                option.tone = toneFromString(trimCopy(item.substr(equals + 1)));
            }
            options.push_back(option);
        } else {
            current += spec[i];
        }
    }
    return options;
}

Column readColumn(const Node& node) {
    Column column;
    column.id = node.id;
    column.label = node.get("label", node.id);
    column.type = node.get("type", "line");
    column.help = node.get("help");
    column.width = node.getInt("width", 1);
    column.hidden = node.get("hidden") == "yes";
    if (node.has("options")) column.options = parseOptions(node.get("options"));
    return column;
}

Field readField(const Node& node) {
    Field field;
    field.id = node.id;
    field.label = node.get("label", node.id);
    field.type = node.get("type", "text");
    field.origin = node.get("origin", "standard");
    field.view = node.get("view");
    field.source = node.get("source");
    field.why = node.get("why");
    field.ifBlank = node.get("if_blank");
    field.example = node.get("example");
    field.placeholder = node.get("placeholder");
    field.groupBy = node.get("group_by");
    field.titleColumn = node.get("title_column");
    field.subtitleColumn = node.get("subtitle_column");
    field.toneColumn = node.get("tone_column");
    field.outlineColumn = node.get("outline_column");
    field.startColumn = node.get("start_column");
    field.finishColumn = node.get("finish_column");
    field.progressColumn = node.get("progress_column");
    field.rowsColumn = node.get("rows_column");
    field.colsColumn = node.get("cols_column");
    if (node.has("options")) field.options = parseOptions(node.get("options"));
    for (const Node* child : node.childrenWithTag("column"))
        field.columns.push_back(readColumn(*child));
    return field;
}

} // namespace

Tone toneFromString(const std::string& name) {
    if (name == "accent") return Tone::Accent;
    if (name == "ok") return Tone::Ok;
    if (name == "warn") return Tone::Warn;
    if (name == "danger") return Tone::Danger;
    if (name == "faint") return Tone::Faint;
    return Tone::Neutral;
}

const Column* Field::column(const std::string& wanted) const {
    for (const auto& column : columns)
        if (column.id == wanted) return &column;
    return nullptr;
}

const Field* Screen::field(const std::string& wanted) const {
    for (const auto& field : fields)
        if (field.id == wanted) return &field;
    return nullptr;
}

bool Definitions::loadDirectory(const std::string& path, std::string& error) {
    namespace fs = std::filesystem;
    screens_.clear();

    std::error_code code;
    if (!fs::is_directory(path, code)) {
        error = "no definitions directory at " + path;
        return false;
    }

    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(path, code)) {
        if (entry.is_regular_file() && entry.path().extension() == ".pmdef")
            files.push_back(entry.path().string());
    }
    std::sort(files.begin(), files.end());

    for (const std::string& file : files) {
        std::string text;
        if (!readFile(file, text)) {
            error = "could not read " + file;
            return false;
        }
        Node root;
        std::string parseError;
        if (!parseBlocks(text, root, parseError)) {
            error = file + ": " + parseError;
            return false;
        }

        Screen screen;
        screen.id = root.get("screen");
        if (screen.id.empty()) {
            error = file + ": missing the screen: key";
            return false;
        }
        screen.title = root.get("title", screen.id);
        screen.phase = root.get("phase", "Other");
        screen.process = root.get("process");
        screen.intro = root.get("intro");
        screen.source = root.get("source");
        screen.order = root.getInt("order", 0);
        for (const Node* child : root.childrenWithTag("field"))
            screen.fields.push_back(readField(*child));
        screens_.push_back(std::move(screen));
    }

    std::stable_sort(screens_.begin(), screens_.end(),
                     [](const Screen& a, const Screen& b) { return a.order < b.order; });

    if (screens_.empty()) {
        error = "no .pmdef files in " + path;
        return false;
    }
    return true;
}

const Screen* Definitions::screen(const std::string& wanted) const {
    for (const auto& screen : screens_)
        if (screen.id == wanted) return &screen;
    return nullptr;
}

std::vector<std::string> Definitions::phases() const {
    std::vector<std::string> found;
    for (const auto& screen : screens_) {
        if (std::find(found.begin(), found.end(), screen.phase) == found.end())
            found.push_back(screen.phase);
    }
    return found;
}

} // namespace pm
