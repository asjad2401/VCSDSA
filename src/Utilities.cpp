#include "../include/Utilities.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>

std::string Utilities::generateUUID() 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 15);
    std::uniform_int_distribution<> dist3(8, 11);

    std::stringstream ss;
    for (int i = 0; i < 8; ++i) ss << std::hex << dist(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << std::hex << dist(gen);
    ss << "-4";
    for (int i = 0; i < 3; ++i) ss << std::hex << dist(gen);
    ss << "-";
    ss << std::hex << dist3(gen);
    for (int i = 0; i < 3; ++i) ss << std::hex << dist(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << std::hex << dist(gen);

    return ss.str();
}

std::string Utilities::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
