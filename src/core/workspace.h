// The folder of project records, and the definitions that describe them.
//
// One file per project, in a folder the user can open, back up and diff. The
// core does not know where that folder is: the front end resolves it and hands
// it in, so the core stays free of any platform or toolkit.
#pragma once

#include "definition.h"
#include "record.h"

#include <memory>
#include <string>
#include <vector>

namespace pm {

struct ProjectSummary {
    std::string id;
    std::string name;
    std::string phase;
    std::string modified;
    std::string accent;
    std::string repo;
    std::string path;
};

class Workspace {
public:
    bool open(const std::string& definitionsDir, const std::string& projectsDir, std::string& error);

    const Definitions& definitions() const { return definitions_; }
    const std::string& projectsDir() const { return projectsDir_; }

    std::vector<ProjectSummary> list() const;

    std::shared_ptr<Record> create(const std::string& name, const std::string& accent, std::string& error);
    std::shared_ptr<Record> load(const std::string& path, std::string& error);
    bool remove(const std::string& path, std::string& error);

    std::string pathFor(const std::string& id) const;

private:
    Definitions definitions_;
    std::string projectsDir_;
};

// Where a fresh install finds its definitions: next to the executable first,
// then the source tree, so a developer build and an installed copy both work.
std::vector<std::string> definitionSearchPaths(const std::string& executableDir);

} // namespace pm
