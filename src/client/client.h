#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <iomanip>
#include <netdb.h>

#include "../P2P/P2PEvent.h"
#include "../P2P/P2PState.h"
#include "../util/MessageParser.h"

class P2PClient {
public:
    struct ClientConfig {
        std::string name;
        std::string server_ip;
        uint16_t server_port;
        uint16_t udp_port;
        uint16_t tcp_port;
    };

    explicit P2PClient(const ClientConfig& config)
        : name_(config.name),
          server_ip_(config.server_ip),
          server_port_(config.server_port),
          udp_port_(config.udp_port),
          tcp_port_(config.tcp_port),
          current_state_(P2PStateType::UNREGISTERED),
          running_(false),
          next_request_number_(1) { setupSocket(); }

    ~P2PClient();

    void start();

    void stop();

    bool registerWithServer();

    bool deregister();

    bool searchItem(const std::string& item_name, const std::string& description, double max_price);

    bool negotiateOffer(int request_number, const std::string& item_name, double counter_price);

    bool acceptOffer(int request_number, const std::string& item_name, double price);

    bool refuseOffer(int request_number, const std::string& item_name, double price);

    static ClientConfig parseCommandLine(int argc, char* argv[]) {
        if (argc != 6) {
            std::cerr << "Usage: " << argv[0]
                << " <client_name> <server_ip> <server_port> <tcp_port>" << std::endl;
            std::cerr << "Example: " << argv[0]
                << " peer1 127.0.0.1 8080 5000 5001" << std::endl;
            throw std::runtime_error("Invalid number of arguments");
        }

        ClientConfig config;
        try {
            config.name = argv[1];
            config.server_ip = argv[2];
            config.server_port = static_cast<uint16_t>(std::stoi(argv[3]));
            config.udp_port = static_cast<uint16_t>(std::stoi(argv[4]));
            config.tcp_port = static_cast<uint16_t>(std::stoi(argv[5]));

            // Validate ports
            if (config.server_port <= 0 || config.server_port > 65535 ||
                config.tcp_port <= 0 || config.tcp_port > 65535) {
                throw std::runtime_error("Port numbers must be between 1 and 65535");
            }

            // Validate tcp and udp ports are not same
            if (config.tcp_port == config.udp_port) { throw std::runtime_error("TCP and UDP ports must be different"); }
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Error parsing arguments: ") + e.what());
        }

        return config;
    }

private:
    std::string name_;
    std::string server_ip_;
    uint16_t server_port_;
    uint16_t udp_port_;
    uint16_t tcp_port_;
    int client_socket_;
    P2PStateType current_state_;
    std::atomic<bool> running_;
    std::atomic<int> next_request_number_;
    std::thread receive_thread_;

    struct Item {
        std::string name;
        std::string description;
        double price;
    };

    std::vector<Item> inventory_;

    void handleSearchEvent(const std::shared_ptr<P2PEvent>& event);

    void addItem(const std::string& name, const std::string& description, double price);

    void removeItem(const std::string& name);

    void listInventory();

    void setupSocket();

    void startCommandLoop();

    void processUserCommand(const std::string& command);

    void receiveMessages();

    void printHelp();

    void printStatus();

    bool sendMessage(const json& msg);

    std::string getLocalIpAddress();

    uint16_t getLocalPort();

    int getNextRequestNumber();

    std::string getStateName(P2PStateType state);

    void logOutgoingMessage(const json& msg);

    // Command handlers
    void handleSearchCommand(std::istringstream& iss);

    void handleNegotiateCommand(std::istringstream& iss);

    void handleAcceptCommand(std::istringstream& iss);

    void handleRefuseCommand(std::istringstream& iss);

    void handleReceivedMessage(const std::string& message);

    void handleOffer(const std::shared_ptr<P2PEvent>& event);
};
