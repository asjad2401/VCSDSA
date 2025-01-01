#include "../include/MergeHandler.h"
#include "../include/FileSystem.h"
#include "../include/Utilities.h"
#include <iostream>
using namespace std;
// Find the common ancestor commit between two branches
std::string MergeHandler::findCommonAncestor(const std::string& branch1, const std::string& branch2) {
    // Load commit timelines for both branches
    std::string branch1Path = ".vcs/branches/" + branch1 + ".json";
    std::string branch2Path = ".vcs/branches/" + branch2 + ".json";

    cout<<branch1Path<<endl<<branch2Path<<endl;
    if (!FileSystem::fileExists(branch1Path) || !FileSystem::fileExists(branch2Path)) {
        std::cerr << "Error: One or both branches do not exist!" << std::endl;
        return "";
    }

    auto branch1Data = nlohmann::json::parse(FileSystem::readFile(branch1Path));
    auto branch2Data = nlohmann::json::parse(FileSystem::readFile(branch2Path));

    const std::vector<std::string>& commits1 = branch1Data["commits"];
    const std::vector<std::string>& commits2 = branch2Data["commits"];

    // Find the last common commit in the two histories
    for (auto it1 = commits1.rbegin(); it1 != commits1.rend(); ++it1) {
        for (auto it2 = commits2.rbegin(); it2 != commits2.rend(); ++it2) {
            if (*it1 == *it2) {
                return *it1;
            }
        }
    }

    return ""; // No common ancestor found
}

// Perform a three-way merge between base, branch1, and branch2 JSON trees
nlohmann::json MergeHandler::threeWayMerge(
    const nlohmann::json& base, 
    const nlohmann::json& branch1, 
    const nlohmann::json& branch2
) {
    nlohmann::json merged = base;

    for (const auto& [key, baseValue] : base.items()) {
        const auto& branch1Value = branch1.contains(key) ? branch1[key] : nlohmann::json();
        const auto& branch2Value = branch2.contains(key) ? branch2[key] : nlohmann::json();

        if (branch1Value == branch2Value) {
            // No conflict, use branch1 (or branch2 since they're identical)
            merged[key] = branch1Value;
        } else if (branch1Value != baseValue && branch2Value == baseValue) {
            // Change only in branch1
            merged[key] = branch1Value;
        } else if (branch2Value != baseValue && branch1Value == baseValue) {
            // Change only in branch2
            merged[key] = branch2Value;
        } else {
            // Conflict: use branch1's value, but could prompt the user to resolve manually
            std::cerr << "Conflict detected for key: " << key << std::endl;
            merged[key] = branch1Value; // Default to branch1
        }
    }

    // Handle keys added in branch1 or branch2 but missing in base
    for (const auto& [key, value] : branch1.items()) {
        if (!base.contains(key)) {
            merged[key] = value;
        }
    }

    for (const auto& [key, value] : branch2.items()) {
        if (!base.contains(key) && !branch1.contains(key)) {
            merged[key] = value;
        }
    }

    return merged;
}
