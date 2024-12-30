#include "../include/FileSystem.h"
#include <filesystem>
#include <fstream>
#include <sstream>

#include "../picosha2.h"

namespace fs = std::filesystem;

bool FileSystem::createDirectory(const std::string& path) {
    return fs::create_directories(path);
}

bool FileSystem::fileExists(const std::string& path) {
    return fs::exists(path);
}

std::string FileSystem::calculateHash(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return "";
    std::ostringstream oss;
    oss << file.rdbuf();
    return picosha2::hash256_hex_string(oss.str());
}

nlohmann::json FileSystem::getDirectoryTree(const std::string& directoryPath) {
    nlohmann::json tree = nlohmann::json::object();
    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            tree[entry.path().string()] = calculateHash(entry.path().string());
        }
    }
    return tree;
}

std::string FileSystem::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) return "";
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool FileSystem::writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file) return false;
    file << content;
    return true;
}

bool FileSystem::copyFile(const std::string& source, const std::string& destination) {
    try {
        fs::copy(source, destination, fs::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}
