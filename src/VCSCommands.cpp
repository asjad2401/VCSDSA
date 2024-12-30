#include "../include/VCSCommands.h"
#include "../include/FileSystem.h"
#include "../include/Utilities.h"
#include "../include/CommitGraph.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <string.h>
#include <fstream>


namespace fs = std::filesystem;
using namespace std;

void VCSCommands::init()
{
    std::string vcsPath = ".vcs";
    FileSystem::createDirectory(vcsPath + "/current_branch");
    FileSystem::createDirectory(vcsPath + "/latest_commit");
    FileSystem::createDirectory(vcsPath + "/staging/files");
    FileSystem::createDirectory(vcsPath + "/staging/tree");
    FileSystem::createDirectory(vcsPath + "/branches");
    FileSystem::createDirectory(vcsPath + "/commits");
    FileSystem::createDirectory(vcsPath + "/data/hash");

    std::cout << "Initialized empty VCS repository in " << vcsPath << std::endl;
}

void VCSCommands::add(const std::string &filePath)
{
    if (filePath == "all")
    {
        // Add all files in the working directory, excluding `.vcs/` and `vcs.exe`
        for (const auto &entry : std::filesystem::recursive_directory_iterator("."))
        {
            std::string entryPath = std::filesystem::relative(entry.path()).string();

            // Skip `.vcs/` directory, `vcs.exe`, and ensure it's a regular file
            if (entryPath.starts_with(".vcs") || entryPath == "vcs.exe" || !entry.is_regular_file())
            {
                continue;
            }

            // Add valid files recursively
            add(entryPath);
        }
    }
    else
    {
        // Add a specific file
        // Skip `.vcs/` directory and `vcs.exe`
        if (filePath.starts_with(".vcs") || filePath == "vcs.exe")
        {
            std::cout << "Skipping file: " << filePath << std::endl;
            return;
        }

        // Calculate the hash and stage the file
        std::string hash = FileSystem::calculateHash(filePath);
        std::string stagePath = ".vcs/staging/files/" + hash;

        // Create the staging directory for this file
        FileSystem::createDirectory(stagePath);
        FileSystem::copyFile(filePath, stagePath + "/" + std::filesystem::path(filePath).filename().string());

        // Save metadata for the file
        nlohmann::json metadata;
        metadata["name"] = std::filesystem::path(filePath).filename().string();
        metadata["hash"] = hash;
        FileSystem::writeFile(stagePath + "/metadata.json", metadata.dump(4));

        std::cout << "Added " << filePath << " to the staging area." << std::endl;
    }

    // Save the directory tree (excluding `.vcs/` and `vcs.exe`)
    nlohmann::json directoryTree = FileSystem::getDirectoryTree(".");
    for (auto it = directoryTree.begin(); it != directoryTree.end();)
    {
        if (it.key().starts_with(".vcs") || it.key() == "vcs.exe")
        {
            it = directoryTree.erase(it); // Remove `.vcs/` and `vcs.exe` entries from the tree
        }
        else
        {
            ++it;
        }
    }

    std::string stageTreePath = ".vcs/staging/tree/staging_tree.json";
    FileSystem::writeFile(stageTreePath, directoryTree.dump(4));

    std::cout << "Saved the current directory tree." << std::endl;
}

void VCSCommands::commit(const std::string &message)
{
    // Generate a unique commit ID
    std::string commitId = Utilities::generateUUID();

    // Determine the current branch or initialize the repository with "master" if no branch exists
    std::string currentBranchPath = ".vcs/current_branch/current_branch.json";
    std::string branchName;
    std::string parentCommitId = "null";

    if (!FileSystem::fileExists(currentBranchPath))
    {
        // No active branch, create the master branch
        branchName = "master";

        // Initialize the master branch in `.vcs/branches/`
        nlohmann::json masterBranch;
        masterBranch["branch_name"] = branchName;
        masterBranch["head"] = "";    // No head commit yet
        masterBranch["commits"] = {}; // Empty commit list
        FileSystem::writeFile(".vcs/branches/" + branchName + ".json", masterBranch.dump(4));

        // Set master as the current branch
        nlohmann::json currentBranch;
        currentBranch["name"] = branchName;
        currentBranch["head"] = ""; // No head commit yet
        FileSystem::writeFile(currentBranchPath, currentBranch.dump(4));

        std::cout << "Initialized repository with master branch." << std::endl;
    }
    else
    {
        // Read the current active branch
        nlohmann::json currentBranch = nlohmann::json::parse(FileSystem::readFile(currentBranchPath));
        branchName = currentBranch["name"];
        parentCommitId = currentBranch["head"];
    }

    // Gather metadata from the working directory (not staging)
    nlohmann::json directoryTree = FileSystem::getDirectoryTree(".");
    for (auto it = directoryTree.begin(); it != directoryTree.end();)
    {
        if (it.key().find(".vcs") != std::string::npos)
        {
            it = directoryTree.erase(it); // Remove `.vcs/` entries
        }
        else
        {
            ++it;
        }
    }

    // Prepare file names and hashes
    std::vector<std::string> fileNames;
    std::vector<std::string> fileHashes;

    for (const auto &entry : std::filesystem::directory_iterator(".vcs/staging/files"))
    {
        if (!entry.is_directory())
            continue; // Only consider directories (file hashes)

        std::string hash = entry.path().filename().string();
        std::string metadataPath = entry.path().string() + "/metadata.json";
        nlohmann::json metadata = nlohmann::json::parse(FileSystem::readFile(metadataPath));
        std::string filePath = entry.path().string() + "/" + metadata["name"].get<std::string>();

        fileNames.push_back(metadata["name"].get<std::string>());
        fileHashes.push_back(hash);

        // Create a folder for the hash in `.vcs/data/hash/`
        std::string hashFolderPath = ".vcs/data/hash/" + hash;
        FileSystem::createDirectory(hashFolderPath);

        // Save the JSON metadata
        nlohmann::json dataEntry;
        dataEntry["file_name"] = metadata["name"];
        dataEntry["file_hash"] = hash;
        dataEntry["branches"] = {};   // Initialize an empty list of branches
        dataEntry["commit_ids"] = {}; // Initialize an empty list of commit IDs

        if (FileSystem::fileExists(hashFolderPath + "/hash.json"))
        {
            // Update existing data entry
            dataEntry = nlohmann::json::parse(FileSystem::readFile(hashFolderPath + "/hash.json"));
        }
        if (std::find(dataEntry["branches"].begin(), dataEntry["branches"].end(), branchName) == dataEntry["branches"].end())
        {
            dataEntry["branches"].push_back(branchName);
        }
        dataEntry["commit_ids"].push_back(commitId);

        FileSystem::writeFile(hashFolderPath + "/hash.json", dataEntry.dump(4));

        // Copy the file itself into the hash folder
        FileSystem::copyFile(filePath, hashFolderPath + "/" + metadata["name"].get<std::string>());
    }

    // Create commit object
    nlohmann::json commit;
    commit["commit_id"] = commitId;
    commit["branch_name"] = branchName; // Use the current branch
    commit["parent"] = parentCommitId; 
    commit["directory_tree"] = directoryTree;
    commit["file_names"] = fileNames;
    commit["file_hashes"] = fileHashes;
    commit["message"] = message;                            // Add commit message
    commit["timestamp"] = Utilities::getCurrentTimestamp(); // Add timestamp

    // Save the commit object
    std::string commitPath = ".vcs/commits/" + commitId + ".json";
    FileSystem::writeFile(commitPath, commit.dump(4));

    // Update the branch
    std::string branchPath = ".vcs/branches/" + branchName + ".json";
    nlohmann::json branchData;
    if (FileSystem::fileExists(branchPath))
    {
        branchData = nlohmann::json::parse(FileSystem::readFile(branchPath));
        branchData["head"] = commitId;
        branchData["commits"].push_back(commitId);
    }
    else
    {
        branchData["branch_name"] = branchName;
        branchData["head"] = commitId;
        branchData["commits"] = {commitId};
    }
    FileSystem::writeFile(branchPath, branchData.dump(4));

    // Update the current branch file
    nlohmann::json currentBranch;
    currentBranch["name"] = branchName;
    currentBranch["head"] = commitId;
    FileSystem::writeFile(currentBranchPath, currentBranch.dump(4));

    // Update the latest commit
    nlohmann::json latestCommit;
    latestCommit["commit_id"] = commitId;
    latestCommit["timestamp"] = Utilities::getCurrentTimestamp();
    FileSystem::writeFile(".vcs/latest_commit/latest_commit.json", latestCommit.dump(4));

    // Clear the staging area
    std::filesystem::remove_all(".vcs/staging/files");              // Remove all staged files
    std::filesystem::create_directory(".vcs/staging/files");        // Recreate the directory
    std::filesystem::remove(".vcs/staging/tree/staging_tree.json"); // Remove tree file

    std::cout << "Committed changes with ID: " << commitId << std::endl;
    std::cout << "Branch: " << branchName << " updated. Staging area cleared." << std::endl;
}

void VCSCommands::branch(const std::string &branchName)
{
    // Path to the current branch metadata file
    std::string currentBranchPath = ".vcs/current_branch/current_branch.json";

    // Ensure the current branch exists
    if (!FileSystem::fileExists(currentBranchPath))
    {
        std::cerr << "Error: No repository initialized or no active branch!" << std::endl;
        return;
    }

    // Read the current branch metadata
    nlohmann::json currentBranch = nlohmann::json::parse(FileSystem::readFile(currentBranchPath));
    std::string currentBranchName = currentBranch["name"];
    std::string currentBranchHead = currentBranch["head"];

    // Path to the current branch file in `.vcs/branches/`
    std::string currentBranchDataPath = ".vcs/branches/" + currentBranchName + ".json";

    // Ensure the current branch data exists
    if (!FileSystem::fileExists(currentBranchDataPath))
    {
        std::cerr << "Error: Current branch data not found in .vcs/branches/!" << std::endl;
        return;
    }

    // Read the current branch data
    nlohmann::json currentBranchData = nlohmann::json::parse(FileSystem::readFile(currentBranchDataPath));

    // Create a new branch metadata object
    nlohmann::json newBranch;
    newBranch["branch_name"] = branchName;
    newBranch["head"] = currentBranchHead;
    newBranch["commits"] = currentBranchData["commits"]; // Copy commit history

    // Save the new branch metadata to `.vcs/branches/`
    std::string newBranchPath = ".vcs/branches/" + branchName + ".json";
    if (FileSystem::fileExists(newBranchPath))
    {
        std::cerr << "Error: Branch \"" << branchName << "\" already exists!" << std::endl;
        return;
    }
    FileSystem::writeFile(newBranchPath, newBranch.dump(4));

    // Update `.vcs/current_branch/` to reflect the new active branch
    nlohmann::json updatedCurrentBranch;
    updatedCurrentBranch["name"] = branchName;
    updatedCurrentBranch["head"] = currentBranchHead;
    FileSystem::writeFile(currentBranchPath, updatedCurrentBranch.dump(4));

    std::cout << "Created a new branch: " << branchName << " and set it as the current branch." << std::endl;
}

void VCSCommands::checkout(const std::string &branchName)
{
    try
    {
        // Path to the target branch file
        std::string branchPath = ".vcs/branches/" + branchName + ".json";

        // Check if the branch exists
        if (!FileSystem::fileExists(branchPath))
        {
            throw std::runtime_error("Branch '" + branchName + "' does not exist!");
        }

        // Read the target branch metadata
        nlohmann::json targetBranch = nlohmann::json::parse(FileSystem::readFile(branchPath));

        if (!targetBranch.contains("head"))
        {
            throw std::runtime_error("Invalid branch format: missing 'head' field!");
        }

        // Retrieve the latest commit ID for the target branch
        std::string commitId = targetBranch["head"];

        // Path to the latest commit file
        std::string commitPath = ".vcs/commits/" + commitId + ".json";
        if (!FileSystem::fileExists(commitPath))
        {
            throw std::runtime_error("Head commit for branch '" + branchName + "' does not exist!");
        }

        // Read the commit metadata
        nlohmann::json commitData = nlohmann::json::parse(FileSystem::readFile(commitPath));

        if (!commitData.contains("directory_tree"))
        {
            throw std::runtime_error("Invalid commit format: missing 'directory_tree'!");
        }

        // Extract the directory tree
        auto directoryTree = commitData["directory_tree"];

        // Clear the working directory (ignoring .vcs folder)
        for (const auto &entry : std::filesystem::directory_iterator("."))
        {
            const auto &path = entry.path().filename().string();
            if (path != ".vcs")
            {
                try
                {
                    std::filesystem::remove_all(entry.path());
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    // std::cerr << "Warning: Could not remove " << entry.path() << ": " << e.what() << std::endl;
                }
            }
        }

        // Restore files from the commit's directory tree
        for (const auto &[filePath, fileHash] : directoryTree.items())
        {
            // Skip .vcs directory entries
            if (filePath.find(".vcs") != std::string::npos)
            {
                continue;
            }

            std::string fileHashStr = fileHash.get<std::string>();

            // Fix the path:
            std::string normalizedPath = filePath;
            if (normalizedPath.substr(0, 2) == ".\\" || normalizedPath.substr(0, 2) == "./")
            {
                normalizedPath = normalizedPath.substr(2);
            }
            std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

            // First ensure parent directory exists
            auto parentPath = std::filesystem::path(normalizedPath).parent_path();
            if (!parentPath.empty())
            {
                std::filesystem::create_directories(parentPath);
            }

            std::string hashDirPath = ".vcs/data/hash/" + fileHashStr;
            try
            {
                for (const auto &entry : std::filesystem::directory_iterator(hashDirPath))
                {
                    if (entry.is_regular_file() && entry.path().filename() != "hash.json")
                    {
                        std::ifstream srcFile(entry.path(), std::ios::binary);
                        if (!srcFile.is_open())
                        {
                            throw std::runtime_error("Could not open source file: " + entry.path().string());
                        }

                        std::ofstream dstFile(normalizedPath, std::ios::binary | std::ios::trunc);
                        if (!dstFile.is_open())
                        {
                            throw std::runtime_error("Could not create destination file: " + normalizedPath);
                        }

                        dstFile << srcFile.rdbuf();
                        std::cout << "Restored: " << normalizedPath << std::endl;
                        break;
                    }
                }
            }
            catch (const std::exception &e)
            {
                // std::cerr << "Warning: Failed to restore file " << normalizedPath << ": " << e.what() << std::endl;
            }
        }

        // Handle special case for vcs.exe
        std::string vcsExePath = "./vcs.exe";
        if (!directoryTree.contains(".\\vcs.exe") && !directoryTree.contains("./vcs.exe"))
        {
            // Remove vcs.exe if it exists
            if (std::filesystem::exists(vcsExePath))
            {
                try
                {
                    std::filesystem::remove(vcsExePath);
                    std::cout << "Removed: vcs.exe (not present in target branch)" << std::endl;
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    // std::cerr << "Warning: Could not remove vcs.exe: " << e.what() << std::endl;
                }
            }
        }

        // Update current branch metadata
        nlohmann::json currentBranch;
        currentBranch["name"] = branchName;
        currentBranch["head"] = commitId;

        FileSystem::writeFile(".vcs/current_branch/current_branch.json", currentBranch.dump(4));

        std::cout << "Successfully switched to branch '" << branchName << "'" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during checkout: " << e.what() << std::endl;
        throw;
    }
}

void VCSCommands::revert(const std::string &commitId)
{
    try
    {
        // Path to the target commit file
        std::string commitPath = ".vcs/commits/" + commitId + ".json";

        // Check if the commit exists
        if (!FileSystem::fileExists(commitPath))
        {
            throw std::runtime_error("Commit '" + commitId + "' does not exist!");
        }

        // Read the target commit metadata
        nlohmann::json commitData = nlohmann::json::parse(FileSystem::readFile(commitPath));

        if (!commitData.contains("directory_tree"))
        {
            throw std::runtime_error("Invalid commit format: missing 'directory_tree'!");
        }

        // Extract the directory tree
        auto directoryTree = commitData["directory_tree"];

        // Clear the working directory (ignoring .vcs folder)
        for (const auto &entry : std::filesystem::directory_iterator("."))
        {
            const auto &path = entry.path().filename().string();
            if (path != ".vcs")
            {
                try
                {
                    std::filesystem::remove_all(entry.path());
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    // std::cerr << "Warning: Could not remove " << entry.path() << ": " << e.what() << std::endl;
                }
            }
        }

        // Restore files from the commit's directory tree
        for (const auto &[filePath, fileHash] : directoryTree.items())
        {
            // Skip .vcs directory entries
            if (filePath.find(".vcs") != std::string::npos)
            {
                continue;
            }

            std::string fileHashStr = fileHash.get<std::string>();

            // Fix the path:
            std::string normalizedPath = filePath;
            if (normalizedPath.substr(0, 2) == ".\\" || normalizedPath.substr(0, 2) == "./")
            {
                normalizedPath = normalizedPath.substr(2);
            }
            std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

            // First ensure parent directory exists
            auto parentPath = std::filesystem::path(normalizedPath).parent_path();
            if (!parentPath.empty())
            {
                std::filesystem::create_directories(parentPath);
            }

            std::string hashDirPath = ".vcs/data/hash/" + fileHashStr;
            try
            {
                for (const auto &entry : std::filesystem::directory_iterator(hashDirPath))
                {
                    if (entry.is_regular_file() && entry.path().filename() != "hash.json")
                    {
                        std::ifstream srcFile(entry.path(), std::ios::binary);
                        if (!srcFile.is_open())
                        {
                            // throw std::runtime_error("Could not open source file: " + entry.path().string());
                        }

                        std::ofstream dstFile(normalizedPath, std::ios::binary | std::ios::trunc);
                        if (!dstFile.is_open())
                        {
                            // throw std::runtime_error("Could not create destination file: " + normalizedPath);
                        }

                        dstFile << srcFile.rdbuf();
                        std::cout << "Restored: " << normalizedPath << std::endl;

                        // Stage the restored file
                        VCSCommands::add(normalizedPath);
                        break;
                    }
                }
            }
            catch (const std::exception &e)
            {
                // std::cerr << "Warning: Failed to restore file " << normalizedPath << ": " << e.what() << std::endl;
            }
        }

        // Handle special case for vcs.exe
        std::string vcsExePath = "./vcs.exe";
        if (!directoryTree.contains(".\\vcs.exe") && !directoryTree.contains("./vcs.exe"))
        {
            // Remove vcs.exe if it exists
            if (std::filesystem::exists(vcsExePath))
            {
                try
                {
                    std::filesystem::remove(vcsExePath);
                    std::cout << "Removed: vcs.exe (not present in target commit)" << std::endl;
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    // std::cerr << "Warning: Could not remove vcs.exe: " << e.what() << std::endl;
                }
            }
        }
        std::string currentBranchPath = ".vcs/current_branch/current_branch.json";
         if (FileSystem::fileExists(currentBranchPath))
        {
            nlohmann::json currentBranch = nlohmann::json::parse(FileSystem::readFile(currentBranchPath));
            currentBranch["head"] = commitId;

            // Save the updated branch data back to the file
            FileSystem::writeFile(currentBranchPath, currentBranch.dump(4));
            std::cout << "Updated current branch head to commit '" << commitId << "'." << std::endl;
        }

        std::cout << "Successfully reverted to commit '" << commitId << "' and staged all changes." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during revert: " << e.what() << std::endl;
        throw;
    }
}

void VCSCommands::merge(const std::string &sourceBranch)
{
    // Paths to branch metadata
    std::string sourceBranchPath = ".vcs/branches/" + sourceBranch + ".json";
    std::string currentBranchPath = ".vcs/current_branch/current_branch.json";


    // Validate source branch
    if (!FileSystem::fileExists(sourceBranchPath)) {
        std::cerr << "Error: Branch '" << sourceBranch << "' does not exist!" << std::endl;
        return;
    }

    // Load branch metadata
    nlohmann::json sourceBranchData, currentBranchData;
    try
    {
        // Read source branch JSON file
        std::ifstream sourceFile(sourceBranchPath);
        if (!sourceFile)
        {
            std::cerr << "Error: Could not open source branch file: " << sourceBranchPath << std::endl;
            return;
        }
        sourceFile >> sourceBranchData; // Parse directly into JSON object
        sourceFile.close();

        // Read current branch JSON file
        std::ifstream currentFile(currentBranchPath);
        if (!currentFile)
        {
            std::cerr << "Error: Could not open current branch file: " << currentBranchPath << std::endl;
            return;
        }
        currentFile >> currentBranchData; // Parse directly into JSON object
        currentFile.close();
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return;
    }
    catch (const std::exception &e)
    {
        std::cerr << "File Handling Error: " << e.what() << std::endl;
        return;
    }

    // Check if 'vcs.exe' is present in directory tree and skip its comparison

    nlohmann::json sourceTree = sourceBranchData["directory_tree"];
    nlohmann::json currentTree = currentBranchData["directory_tree"];

    // Remove `vcs.exe` from comparison if it exists in either branch
    for (auto it = sourceTree.begin(); it != sourceTree.end();)
    {
        if (it.key() == "./vcs.exe")
        {
            it = sourceTree.erase(it); // Remove `vcs.exe` from the source branch tree
        }
        else
        {
            ++it;
        }
    }

    for (auto it = currentTree.begin(); it != currentTree.end();)
    {
        if (it.key() == "./vcs.exe")
        {
            it = currentTree.erase(it); // Remove `vcs.exe` from the current branch tree
        }
        else
        {
            ++it;
        }
    }


    std::string sourceHead = sourceBranchData["head"];
    std::string currentHead = currentBranchData["head"];
    std::string currentBranchName = currentBranchData["name"];


    // Check if branches are already merged
    if (sourceHead == currentHead)
    {
        std::cout << "Branches are already merged." << std::endl;
        return;
    }

    // Paths to commit metadata
    std::string sourceCommitPath = ".vcs/commits/" + sourceHead + ".json";
    std::string currentCommitPath = ".vcs/commits/" + currentHead + ".json";


    // Validate commits
    if (!FileSystem::fileExists(sourceCommitPath) || !FileSystem::fileExists(currentCommitPath))
    {
        std::cerr << "Error: Commit metadata for one of the branches is missing!" << std::endl;
        return;
    }

    // Load commit metadata
    nlohmann::json sourceCommitData = nlohmann::json::parse(FileSystem::readFile(sourceCommitPath));
    nlohmann::json currentCommitData = nlohmann::json::parse(FileSystem::readFile(currentCommitPath));


    // Merge directory trees
    nlohmann::json mergedTree = currentTree;
    bool conflictDetected = false;

    for (const auto &[filePath, sourceHash] : sourceTree.items())
    {
        if (currentTree.contains(filePath))
        {
            // Detect conflict
            if (currentTree[filePath] != sourceHash)
            {
                conflictDetected = true;
                std::cerr << "Conflict detected in file: " << filePath << std::endl;
            }
        }
        else
        {
            // Add file from source branch
            mergedTree[filePath] = sourceHash;
        }
    }
    cout << "after merging loop" << endl;
    // Abort merge if conflicts detected
    if (conflictDetected)
    {
        std::cerr << "Merge aborted due to conflicts. Resolve them manually." << std::endl;
        return;
    }

    // Write merged directory tree to the staging area
    for (const auto &[filePath, fileHash] : mergedTree.items())
    {
        add(filePath); // Staging files for commit
    }

    // Commit the merge
    std::string mergeMessage = "Merged branch '" + sourceBranch + "' into '" + currentBranchName + "'";
    commit(mergeMessage);

    // Update current branch metadata
    currentBranchData["head"] = sourceHead;
    FileSystem::writeFile(currentBranchPath, currentBranchData.dump(4));

    std::cout << "Successfully merged branch '" << sourceBranch << "' into the current branch." << std::endl;
}

void VCSCommands::log()
{
    // Path to the current branch metadata
    std::string currentBranchPath = ".vcs/current_branch/current_branch.json";

    if (!FileSystem::fileExists(currentBranchPath))
    {
        std::cout << "No branch is currently checked out. Initialize a repository or check out a branch." << std::endl;
        return;
    }

    // Read the current branch information
    nlohmann::json currentBranch = nlohmann::json::parse(FileSystem::readFile(currentBranchPath));
    std::string branchName = currentBranch["name"];

    // Path to the branch file
    std::string branchPath = ".vcs/branches/" + branchName + ".json";

    if (!FileSystem::fileExists(branchPath))
    {
        std::cout << "Branch metadata is missing for the current branch: " << branchName << std::endl;
        return;
    }

    // Read the branch data
    nlohmann::json branchData = nlohmann::json::parse(FileSystem::readFile(branchPath));

    if (!branchData.contains("commits") || branchData["commits"].empty())
    {
        std::cout << "No commits found on the current branch: " << branchName << std::endl;
        return;
    }

    // Fetch commit IDs in reverse order (latest first)
    std::vector<std::string> commitIds = branchData["commits"].get<std::vector<std::string>>();
    std::reverse(commitIds.begin(), commitIds.end());

    std::cout << "Commit history for branch: " << branchName << std::endl;

    for (const std::string &commitId : commitIds)
    {
        // Path to the commit file
        std::string commitPath = ".vcs/commits/" + commitId + ".json";

        if (!FileSystem::fileExists(commitPath))
        {
            std::cout << "Warning: Commit metadata missing for commit ID: " << commitId << std::endl;
            continue;
        }

        // Read the commit metadata
        nlohmann::json commitData = nlohmann::json::parse(FileSystem::readFile(commitPath));

        // Extract details
        std::string timestamp = commitData.value("timestamp", "Unknown");
        std::string message = commitData.value("message", "No message");

        // Print commit details
        std::cout << "Commit ID: " << commitId << std::endl;
        std::cout << "Timestamp: " << timestamp << std::endl;
        std::cout << "Message: " << message << std::endl;
        std::cout << "-------------------------------" << std::endl;
    }
}

void VCSCommands::graph(){
    CommitGraph graph;
    graph.buildGraph(".vcs"); // Build the commit graph
    graph.displayGraph(); // Display the graph

    
    std::ofstream dotFile("commit_graph.dot");
    dotFile << graph.exportToDOT();
    dotFile.close();
    std::cout << "Graph exported to 'commit_graph.dot'. Use Graphviz to visualize.\n";
}