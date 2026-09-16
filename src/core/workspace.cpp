#include "workspace.h"

#include "blockfile.h"

#include <algorithm>
#include <filesystem>

namespace pm {
namespace fs = std::filesystem;

bool Workspace::open(const std::string& definitionsDir, const std::string& projectsDir,
                     std::string& error) {
    if (!definitions_.loadDirectory(definitionsDir, error)) return false;

    std::error_code code;
    fs::create_directories(projectsDir, code);
    if (code) {
        error = "could not create " + projectsDir + ": " + code.message();
        return false;
    }
    projectsDir_ = projectsDir;
    return true;
}

std::vector<ProjectSummary> Workspace::list() const {
    std::vector<ProjectSummary> found;
    std::error_code code;
    if (!fs::is_directory(projectsDir_, code)) return found;

    for (const auto& entry : fs::directory_iterator(projectsDir_, code)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".pmproj") continue;

        // Read only the header: the list does not need every field.
        std::string text;
        if (!readFile(entry.path().string(), text)) continue;
        Node root;
        std::string parseError;
        if (!parseBlocks(text, root, parseError)) continue;

        ProjectSummary summary;
        summary.id = root.get("project", entry.path().stem().string());
        summary.name = root.get("name", summary.id);
        summary.phase = root.get("phase");
        summary.modified = root.get("modified");
        summary.accent = root.get("accent");
        summary.repo = root.get("repo");
        summary.path = entry.path().string();
        found.push_back(std::move(summary));
    }

    std::sort(found.begin(), found.end(), [](const ProjectSummary& a, const ProjectSummary& b) {
        if (a.modified != b.modified) return a.modified > b.modified;
        return a.name < b.name;
    });
    return found;
}

std::string Workspace::pathFor(const std::string& id) const {
    return (fs::path(projectsDir_) / (id + ".pmproj")).string();
}

std::shared_ptr<Record> Workspace::create(const std::string& name, const std::string& accent,
                                          std::string& error) {
    auto record = std::make_shared<Record>();
    record->id = slugify(name);
    // Never silently overwrite an existing project.
    std::string base = record->id;
    int suffix = 2;
    while (fs::exists(pathFor(record->id)))
        record->id = base + "-" + std::to_string(suffix++);

    record->name = name;
    record->accent = accent;
    record->created = todayIso();
    record->modified = record->created;
    record->phase = "Initiating";

    if (!record->save(pathFor(record->id), error)) return nullptr;
    return record;
}

std::shared_ptr<Record> Workspace::load(const std::string& path, std::string& error) {
    auto record = std::make_shared<Record>();
    if (!record->load(path, error)) return nullptr;
    return record;
}

bool Workspace::remove(const std::string& path, std::string& error) {
    std::error_code code;
    fs::remove(path, code);
    if (code) {
        error = code.message();
        return false;
    }
    return true;
}

std::vector<std::string> definitionSearchPaths(const std::string& executableDir) {
    fs::path exe(executableDir);
    return {
        (exe / "definitions").string(),           // installed layout
        (exe / ".." / "definitions").string(),    // build/Release next to the tree
        (exe / ".." / ".." / "definitions").string(),
        (exe / ".." / ".." / ".." / "definitions").string(),
    };
}

} // namespace pm
