#ifndef COMMIT_GRAPH_H
#define COMMIT_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

struct CommitNode {
    std::string commitId;
    std::string message;
    std::string timestamp;
    std::vector<std::string> parents; // Supports multiple parents for merges

    CommitNode(const std::string& id, const std::string& msg, const std::string& time, const std::vector<std::string>& parentIds)
        : commitId(id), message(msg), timestamp(time), parents(parentIds) {}
};


class CommitGraph {
private:
    std::unordered_map<std::string, std::shared_ptr<CommitNode>> nodes; // Commit ID -> CommitNode

    void loadBranch(const std::string& branchFilePath);
    void loadCommit(const std::string& commitId);

public:
    CommitGraph();
    void buildGraph(const std::string& vcsPath);
    void displayGraph(); // Prints the graph for visual inspection
    std::string exportToDOT(); // Exports the graph in DOT format for tools like Graphviz
};

#endif // COMMIT_GRAPH_H
