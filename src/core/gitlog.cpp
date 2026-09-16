#include "gitlog.h"

#include <array>
#include <cstdio>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace pm {
namespace {

// Field and record separators that cannot appear in a commit subject, so the
// output parses without quoting rules.
constexpr char kUnit = '\x1f';
constexpr char kRecord = '\x1e';

// Only a fixed set of shapes is ever run, and the only caller-supplied values
// are a directory and a ref. Both are quoted and screened for the characters
// that would end the argument.
bool safeArgument(const std::string& value) {
    for (char c : value) {
        if (c == '"' || c == '`' || c == '$' || c == '\n' || c == '\r' || c == '%') return false;
    }
    return true;
}

std::string run(const std::string& command) {
    std::string out;
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) return out;
    std::array<char, 4096> buffer{};
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe))
        out += buffer.data();
    PCLOSE(pipe);
    return out;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return std::string();
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string quoted(const std::string& dir) { return "\"" + dir + "\""; }

std::vector<Commit> parse(const std::string& output) {
    std::vector<Commit> commits;
    std::string record;
    for (size_t i = 0; i <= output.size(); ++i) {
        if (i < output.size() && output[i] != kRecord) {
            record += output[i];
            continue;
        }
        const std::string entry = trim(record);
        record.clear();
        if (entry.empty()) continue;

        std::vector<std::string> parts;
        std::string field;
        for (char c : entry) {
            if (c == kUnit) { parts.push_back(field); field.clear(); }
            else field += c;
        }
        parts.push_back(field);
        if (parts.size() < 4) continue;

        Commit commit;
        commit.hash = trim(parts[0]);
        commit.date = trim(parts[1]).substr(0, 10);
        commit.author = trim(parts[2]);
        commit.subject = trim(parts[3]);
        commits.push_back(std::move(commit));
    }
    return commits;
}

} // namespace

bool Git::isRepository(const std::string& dir) {
    if (!safeArgument(dir)) return false;
    const std::string out =
        run("git -C " + quoted(dir) + " rev-parse --is-inside-work-tree 2>&1");
    return trim(out) == "true";
}

std::vector<Commit> Git::log(const std::string& dir, const std::string& sinceIso,
                             const std::string& untilIso, int limit) {
    if (!safeArgument(dir) || !safeArgument(sinceIso) || !safeArgument(untilIso)) return {};
    if (limit < 1) limit = 1;
    if (limit > 2000) limit = 2000;

    std::string command = "git -C " + quoted(dir) + " log --no-merges -n " +
                          std::to_string(limit) +
                          " --pretty=format:%h\x1f%aI\x1f%an\x1f%s\x1e";
    if (!sinceIso.empty()) command += " --since=" + quoted(sinceIso + " 00:00:00");
    // git's --until is exclusive of the day when no time is given, so the end
    // date is carried to the end of that day rather than dropping its commits.
    if (!untilIso.empty()) command += " --until=" + quoted(untilIso + " 23:59:59");
    command += " 2>nul";

    return parse(run(command));
}

Commit Git::show(const std::string& dir, const std::string& ref) {
    if (!safeArgument(dir) || !safeArgument(ref)) return {};
    const std::string command = "git -C " + quoted(dir) + " show -s --pretty=format:%h\x1f%aI\x1f%an\x1f%s\x1e " +
                                quoted(ref) + " 2>nul";
    const std::vector<Commit> found = parse(run(command));
    return found.empty() ? Commit{} : found.front();
}

std::string Git::firstCommitDate(const std::string& dir) {
    if (!safeArgument(dir)) return {};
    const std::string out =
        run("git -C " + quoted(dir) + " log --reverse --pretty=format:%aI -n 1 2>nul");
    return trim(out).substr(0, 10);
}

std::string Git::head(const std::string& dir) {
    if (!safeArgument(dir)) return {};
    return trim(run("git -C " + quoted(dir) + " rev-parse --short HEAD 2>nul"));
}

} // namespace pm
