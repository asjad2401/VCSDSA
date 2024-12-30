#include "../include/VCSCommands.h"
#include <iostream>
#include <string>

void printHelp()
{
    std::cout << "Usage: vcs <command> [arguments]\n";
    std::cout << "Commands:\n";
    std::cout << "  init                        Initialize a new repository\n";
    std::cout << "  add <file>                  Add a file to the staging area\n";
    std::cout << "  commit <message>            Commit changes with a message\n";
    std::cout << "  branch <branch_name>        Create a new branch\n";
    std::cout << "  checkout <branch_name>      Switch to a different branch\n";
    std::cout << "  revert <commit_id>          Revert changes to a specific commit\n";
    std::cout << "  merge <source_branch>       Merge another branch into the current one\n";
    std::cout << "  exit                        Exit the program\n";
    std::cout << "  log                         Show log of commits in current branch\n";
    std::cout << "  graph                       Show Directed Acyclic Graph of commit history\n";
    std::cout << "  -h                          Show this help message\n";
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printHelp();
        return 1; // No command provided, show help
    }

    std::string command = argv[1];

    if (command == "-h")
    {
        printHelp();
        return 0;
    }
    if (command == "init")
    {
        VCSCommands::init();
    }
    else if (command == "add")
    {
        if (argc < 3)
        {
            std::cout << "Usage: vcs add <file>" << std::endl;
            return 1; // Missing file argument
        }
        std::string filePath = argv[2];
        VCSCommands::add(filePath);
    }
    else if (command == "commit")
    {
        if (argc < 3)
        {
            std::cout << "Usage: vcs commit <message>" << std::endl;
            return 1; // Missing commit message
        }
        std::string message = argv[2]; // Use the third argument as the commit message
        VCSCommands::commit(message);
    }
    else if (command == "branch")
    {
        if (argc < 3)
        {
            std::cout << "Usage: vcs branch <branch_name>" << std::endl;
            return 1; // Missing branch name
        }
        std::string branchName = argv[2];
        VCSCommands::branch(branchName);
    }
    else if (command == "checkout")
    {
        if (argc < 3)
        {
            std::cout << "Usage: vcs checkout <branch_name>" << std::endl;
            return 1; // Missing branch name
        }
        std::string branchName = argv[2];
        VCSCommands::checkout(branchName);
    }
    else if (command == "revert")
    {
        if (argc < 3)
        {
            std::cout << "Usage: vcs revert <commit_id>" << std::endl;
            return 1; // Missing commit ID
        }
        std::string commitId = argv[2];
        VCSCommands::revert(commitId);
    }
    else if (command == "merge")
    {
        if (argc < 3)
        {
            std::cout << "Usage: vcs merge <source_branch>" << std::endl;
            return 1; // Missing source branch
        }
        std::string sourceBranch = argv[2];
        VCSCommands::merge(sourceBranch);
    }
    else if (command == "log")
    {
        VCSCommands::log(); // Call the log command
    }
        else if (command == "graph")
    {
        VCSCommands::graph(); // Call the log command
    }
    else if (command == "exit")
    {
        return 0; // Exit the program
    }
    else
    {
        std::cout << "Unknown command: " << command << std::endl;
        printHelp();
        return 1; // Command not recognized
    }

    return 0;
}
