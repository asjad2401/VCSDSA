#ifndef VCS_COMMANDS_H
#define VCS_COMMANDS_H

#include <string>

class VCSCommands {
public:
    static void init();
    static void add(const std::string& filePath);
    static void commit(const std::string& message);
    static void branch(const std::string& branchName);
    static void checkout(const std::string& branchName);
    static void revert(const std::string& commitId);
    static void merge(const std::string& sourceBranch);
    static void log();
    static void graph();

};

#endif // VCS_COMMANDS_H
