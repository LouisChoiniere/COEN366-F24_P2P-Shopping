#ifndef P2P_WITH_EVENTING_MESSAGEPARSER_H
#define P2P_WITH_EVENTING_MESSAGEPARSER_H

// #include <nlohmann/json.hpp>
#include "libraries/json.hpp"
#include "P2PEvent.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

class MessageParser {
public:
    static std::shared_ptr<P2PEvent> parseMessage(const std::string& message);
    static bool validateCommandFields(const json& j, P2PEventType type);
    static void printMessage(const json& j);

private:
    static const std::unordered_map<std::string, P2PEventType> commandMap;
    static void printCommandFields(const json& j, const std::string& command);
    static P2PEventType stringToEventType(const std::string& commandStr);
    static std::string getCurrentTimestamp();
};

#endif //P2P_WITH_EVENTING_MESSAGEPARSER_H