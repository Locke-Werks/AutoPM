// Reading git, for evidence rather than for status.
//
// ProjectMan already reports git state: branch, dirty files, ahead and behind,
// open pull requests. That is an operational feed and this is not it.
//
// AutoPM documents, so it wants git for two things the record cannot get any
// other way. First, evidence: a decision recorded against a commit hash is
// checkable by anyone, where "he said so" is not. Second, a cross-check: the
// board claims a sprint finished ten points, and the commits in that window
// either support that or do not. The tool that insists every claim shows where
// it came from should be able to hold its own claims up to what was actually
// done.
//
// It never writes, never fetches, and never moves a card on its own. Finding
// out that the record disagrees with the history is the PM's business.
#pragma once

#include <string>
#include <vector>

namespace pm {

struct Commit {
    std::string hash;      // short hash
    std::string date;      // yyyy-mm-dd
    std::string author;
    std::string subject;
};

class Git {
public:
    // False when the directory is not a work tree, or git is not installed.
    static bool isRepository(const std::string& dir);

    // Commits touching the given date window, newest first. Empty dates mean
    // unbounded on that side.
    static std::vector<Commit> log(const std::string& dir, const std::string& sinceIso,
                                   const std::string& untilIso, int limit = 200);

    // One commit by ref, for pinning an entry to it. Empty hash means not found.
    static Commit show(const std::string& dir, const std::string& ref);

    // The date of the first commit: when work actually started, as opposed to
    // when the charter says it did.
    static std::string firstCommitDate(const std::string& dir);

    static std::string head(const std::string& dir);
};

} // namespace pm
