#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <nlohmann/json.hpp>

class FileSystem {
public:
    static bool createDirectory(const std::string& path);
    static bool fileExists(const std::string& path);
    static std::string calculateHash(const std::string& filePath);
    static nlohmann::json getDirectoryTree(const std::string& directoryPath);
    static std::string readFile(const std::string& filePath);
    static bool writeFile(const std::string& filePath, const std::string& content);
    static bool copyFile(const std::string& source, const std::string& destination);
};

#endif // FILESYSTEM_H
