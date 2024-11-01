#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include "P2PEvent.h"
#include "State.h"
#include "Statemachine.h"
#include "P2PState.h"
#include "ServerState.h"
#include "PeerStateMachine.h"
#include "ServerStateMachine.h"
#include "ConcurrentQueue.h"
#include "ThreadPool.h"
#include "PeerSession.h"
#include "ConcurrentServer.h"



// Example usage
int main() {
    try {
        // Create server with 4 worker threads
        ConcurrentServer server(5000, 4);

        // Start server
        std::cout << "Server started on port 5000\n";
        server.start();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}