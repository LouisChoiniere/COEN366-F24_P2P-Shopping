#include "client.h"
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        // Parse command line arguments using the static method
        auto config = P2PClient::parseCommandLine(argc, argv);

        // Print configuration
        std::cout << "\n=== P2P Client Configuration ===" << std::endl;
        std::cout << "Client Name: " << config.name << std::endl;
        std::cout << "Server IP: " << config.server_ip << std::endl;
        std::cout << "Server Port: " << config.server_port << std::endl;
        std::cout << "UDP Port: " << config.udp_port << std::endl;
        std::cout << "TCP Port: " << config.tcp_port << std::endl;
        std::cout << "==============================\n" << std::endl;

        // Create and start client
        P2PClient client(config);
        client.start();
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}