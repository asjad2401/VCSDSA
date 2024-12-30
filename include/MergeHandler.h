#ifndef MERGE_HANDLER_H
#define MERGE_HANDLER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class MergeHandler {
public:
    static std::string findCommonAncestor(const std::string& branch1, const std::string& branch2);
    static nlohmann::json threeWayMerge(
        const nlohmann::json& base, 
        const nlohmann::json& branch1, 
        const nlohmann::json& branch2
    );
};

#endif // MERGE_HANDLER_H
