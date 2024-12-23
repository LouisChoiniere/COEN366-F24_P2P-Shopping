#include <iostream>

#include "server/ConcurrentServer.h"

int main(int argc, char *argv[]) {
    try {
        // Create server with 4 worker threads
        ConcurrentServer server(8080, 4);

        // Start server
        std::cout << "Server started on port 8080\n";
        server.start();
    }
    catch (const std::exception &e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
