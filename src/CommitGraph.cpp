#include "../include/CommitGraph.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // Use nlohmann JSON for parsing

using json = nlohmann::json;

CommitGraph::CommitGraph() {}

void CommitGraph::loadBranch(const std::string &branchFilePath)
{
    std::ifstream branchFile(branchFilePath);
    if (!branchFile)
    {
        std::cerr << "Failed to open branch file: " << branchFilePath << std::endl;
        return;
    }

    json branchJson;
    branchFile >> branchJson;

    std::string branchName = branchJson["branch_name"];
    std::vector<std::string> commits = branchJson["commits"];

    for (const std::string &commitId : commits)
    {
        loadCommit(".vcs/commits/" + commitId + ".json");
    }
}

void CommitGraph::loadCommit(const std::string &commitFilePath)
{
    std::ifstream commitFile(commitFilePath);
    if (!commitFile)
    {
        std::cerr << "Failed to open commit file: " << commitFilePath << std::endl;
        return;
    }

    json commitJson;
    commitFile >> commitJson;

    std::string commitId = commitJson["commit_id"];
    std::string message = commitJson["message"];
    std::string timestamp = commitJson["timestamp"];
    std::vector<std::string> parents; // Parents of the commit

    // Add primary parent (if it exists)
    if (commitJson.contains("parent"))
    {
        parents.push_back(commitJson["parent"]);
    }

    // Identify and handle merge commits
    if (message.rfind("Merged branch", 0) == 0)
    { // Check if the message starts with "Merged branch"
        // Parse the source branch name
        size_t pos1 = message.find('\'');
        size_t pos2 = message.find('\'', pos1 + 1);
        if (pos1 != std::string::npos && pos2 != std::string::npos)
        {
            std::string sourceBranch = message.substr(pos1 + 1, pos2 - pos1 - 1);

            // Load source branch head
            std::string sourceBranchPath = ".vcs/branches/" + sourceBranch + ".json";
            std::ifstream sourceFile(sourceBranchPath);
            if (sourceFile.is_open())
            {
                json sourceBranchJson;
                sourceFile >> sourceBranchJson;

                if (sourceBranchJson.contains("head"))
                {
                    parents.push_back(sourceBranchJson["head"]); // Add source branch head as an additional parent
                }
            }
        }
    }

    // Add the commit node to the graph
    auto node = std::make_shared<CommitNode>(commitId, message, timestamp, parents);
    nodes[commitId] = node;
}

void CommitGraph::buildGraph(const std::string &vcsPath)
{
    // Iterate through branch files to load all branches
    for (const auto &entry : std::filesystem::directory_iterator(vcsPath + "/branches"))
    {
        loadBranch(entry.path().string());
    }
}

void CommitGraph::displayGraph()
{
    std::cout << "Commit History Graph:\n";
    for (const auto &[commitId, node] : nodes)
    {
        std::cout << "Commit: " << commitId << "\n";
        std::cout << "Message: " << node->message << "\n";
        std::cout << "Timestamp: " << node->timestamp << "\n";

        if (!node->parents.empty())
        {
            std::cout << "Parents: ";
            for (const auto &parent : node->parents)
            {
                std::cout << parent << " ";
            }
            std::cout << "\n";
        }
        std::cout << "-------------------\n";
    }
}

std::string CommitGraph::exportToDOT()
{
    std::string dot = "digraph CommitGraph {\n";
    for (const auto &[commitId, node] : nodes)
    {
        dot += "    \"" + commitId + "\" [label=\"" + node->message + "\\n" + node->timestamp + "\"];\n";
        for (const auto &parent : node->parents)
        {
            dot += "    \"" + parent + "\" -> \"" + commitId + "\";\n";
        }
    }
    dot += "}\n";
    return dot;
}
