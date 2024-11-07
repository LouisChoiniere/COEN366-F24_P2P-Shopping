#include "MessageParser.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::shared_ptr<P2PEvent> MessageParser::parseMessage(const std::string& message) {
    try {
        auto j = json::parse(message);

        // Print the incoming message
        printMessage(j);

        // Basic validation
        if (!j.contains("command") || !j.contains("rq")) {
            std::cerr << "Missing required fields in message" << std::endl;
            return nullptr;
        }

        // Initialize message data with common fields
        P2PEvent::MessageData data{
            j["rq"].get<int>(),
            j.value("name", "") // Optional for some commands
        };

        // Get command type
        auto type = stringToEventType(j["command"].get<std::string>());

        // Populate command-specific fields
        switch (type) {
        case P2PEventType::REGISTER:
            data.ip_address = j["ip"];
            data.udp_port = j["udp_port"];
            data.tcp_port = j["tcp_port"];
            break;

        case P2PEventType::LOOKING_FOR:
        case P2PEventType::SEARCH:
            data.item_name = j["item_name"];
            data.item_description = j["description"];
            if (type == P2PEventType::LOOKING_FOR) {
                data.max_price = j["max_price"];
            }
            break;

        case P2PEventType::OFFER:
            data.item_name = j["item_name"];
            data.price = j["price"];
            break;

        case P2PEventType::NEGOTIATE:
            data.item_name = j["item_name"];
            data.price = j["max_price"];
            break;

        case P2PEventType::ACCEPT:
        case P2PEventType::REFUSE:
            data.item_name = j["item_name"];
            data.price = j["price"];
            break;

        case P2PEventType::NOT_AVAILABLE:
        case P2PEventType::NOT_FOUND:
        case P2PEventType::FOUND:
            data.item_name = j["item_name"];
            if (j.contains("price")) {
                data.price = j["price"];
            }
            break;

        case P2PEventType::REGISTER_DENIED:
            if (j.contains("reason")) {
                data.reason = j["reason"];
            }
            break;

        default:
            break;
        }

        // Validate fields for this command type
        if (!validateCommandFields(j, type)) {
            return nullptr;
        }

        return std::make_shared<P2PEvent>(type, data);
    }
    catch (const json::exception& e) {
        std::cerr << "JSON error: " << e.what() << std::endl;
        return nullptr;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
        return nullptr;
    }
}

bool MessageParser::validateCommandFields(const json& j, P2PEventType type) {
    try {
        switch (type) {
        case P2PEventType::REGISTER:
            return j.contains("ip") && j.contains("udp_port") && j.contains("tcp_port");

        case P2PEventType::LOOKING_FOR:
            return j.contains("item_name") && j.contains("description") &&
                j.contains("max_price") && j.contains("name");

        case P2PEventType::SEARCH:
            return j.contains("item_name") && j.contains("description");

        case P2PEventType::OFFER:
            return j.contains("item_name") && j.contains("price") && j.contains("name");

        case P2PEventType::NEGOTIATE:
        case P2PEventType::ACCEPT:
        case P2PEventType::REFUSE:
            return j.contains("item_name") && j.contains("price") && j.contains("name");

        case P2PEventType::NOT_AVAILABLE:
        case P2PEventType::NOT_FOUND:
        case P2PEventType::FOUND:
            return j.contains("item_name");

        case P2PEventType::REGISTER_DENIED:
        case P2PEventType::REGISTERED:
            return true; // reason is optional

        case P2PEventType::DE_REGISTER:
            return j.contains("name");

        default:
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Validation error: " << e.what() << std::endl;
        return false;
    }
}

void MessageParser::printMessage(const json& j) {
    static const std::string separator(60, '=');
    static const std::string subseparator(60, '-');

    std::cout << "\n" << separator << std::endl;
    std::cout << "Received Message at: " << getCurrentTimestamp() << std::endl;
    std::cout << subseparator << std::endl;

    const std::string& command = j.value("command", "UNKNOWN");
    std::cout << std::left << std::setw(15) << "Command:" << command << std::endl;
    std::cout << std::left << std::setw(15) << "Request #:" << j.value("rq", -1) << std::endl;

    // Print command-specific fields
    printCommandFields(j, command);

    std::cout << subseparator << std::endl;
    std::cout << "Raw JSON:" << std::endl;
    std::cout << j.dump(4) << std::endl;
    std::cout << separator << "\n" << std::endl;
}

void MessageParser::printCommandFields(const json& j, const std::string& command) {
    if (command == "REGISTER") {
        std::cout << std::left << std::setw(15) << "Name:" << j.value("name", "UNKNOWN") << std::endl;
        std::cout << std::left << std::setw(15) << "IP Address:" << j.value("ip", "UNKNOWN") << std::endl;
        std::cout << std::left << std::setw(15) << "UDP Port:" << j.value("udp_port", -1) << std::endl;
        std::cout << std::left << std::setw(15) << "TCP Port:" << j.value("tcp_port", -1) << std::endl;
    }
    else if (command == "REGISTER-DENIED") {
        std::cout << std::left << std::setw(15) << "Reason:" << j.value("reason", "UNKNOWN") << std::endl;
    }
    else if (command == "LOOKING_FOR" || command == "SEARCH") {
        std::cout << std::left << std::setw(15) << "Item:" << j.value("item_name", "UNKNOWN") << std::endl;
        std::cout << std::left << std::setw(15) << "Description:" << j.value("description", "UNKNOWN") << std::endl;
        if (j.contains("max_price")) {
            std::cout << std::left << std::setw(15) << "Max Price:" << "$" << std::fixed
                << std::setprecision(2) << j["max_price"].get<double>() << std::endl;
        }
    }
    else if (command == "OFFER" || command == "NEGOTIATE" || command == "ACCEPT" ||
        command == "REFUSE" || command == "NOT_AVAILABLE" || command == "NOT_FOUND" ||
        command == "FOUND" || command == "RESERVE" || command == "CANCEL" || command == "BUY") {
        if (j.contains("name")) {
            std::cout << std::left << std::setw(15) << "Name:" << j["name"].get<std::string>() << std::endl;
        }
        std::cout << std::left << std::setw(15) << "Item:" << j.value("item_name", "UNKNOWN") << std::endl;
        std::cout << std::left << std::setw(15) << "Price:" << "$" << std::fixed
            << std::setprecision(2) << j.value("price", 0.0) << std::endl;
    }
}

P2PEventType MessageParser::stringToEventType(const std::string& commandStr) {
    auto it = commandMap.find(commandStr);
    return it != commandMap.end() ? it->second : P2PEventType::UNKNOWN;
}

std::string MessageParser::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_tm = std::localtime(&now_c);

    std::stringstream ss;
    ss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Initialize the command mapping
const std::unordered_map<std::string, P2PEventType> MessageParser::commandMap = {
    {"REGISTER", P2PEventType::REGISTER},
    {"REGISTER-DENIED", P2PEventType::REGISTER_DENIED},
    {"REGISTERED", P2PEventType::REGISTERED},
    {"LOOKING_FOR", P2PEventType::LOOKING_FOR},
    {"SEARCH", P2PEventType::SEARCH},
    {"OFFER", P2PEventType::OFFER},
    {"NOT_AVAILABLE", P2PEventType::NOT_AVAILABLE},
    {"NEGOTIATE", P2PEventType::NEGOTIATE},
    {"ACCEPT", P2PEventType::ACCEPT},
    {"FOUND", P2PEventType::FOUND},
    {"REFUSE", P2PEventType::REFUSE},
    {"NOT_FOUND", P2PEventType::NOT_FOUND},
    {"RESERVE", P2PEventType::RESERVE},
    {"CANCEL", P2PEventType::CANCEL},
    {"BUY", P2PEventType::BUY}
};
