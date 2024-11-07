#include <iostream>

#include "server/ConcurrentServer.h"


// Example usage
int main() {
    std::string mode;
    std::cout << "Start as server or client? ";

    try {
        std::cin >> mode;

        if (mode == "client") {
            std::cout << "Client mode is not implemented yet.\n";
            return 0;
        }
        else if (mode == "server") {
            // Create server with 4 worker threads
            ConcurrentServer server(5000, 4);

            // Start server
            std::cout << "Server started on port 5000\n";
            server.start();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
