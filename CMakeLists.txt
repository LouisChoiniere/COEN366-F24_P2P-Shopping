cmake_minimum_required(VERSION 3.25)
project(P2P_with_Eventing)

set(CMAKE_CXX_STANDARD 20)

# Add pthread library
find_package(Threads REQUIRED)

add_executable(P2P_server main.cpp MessageParser.cpp ThreadPool.cpp ServerHandlers.cpp)

# Link against pthread
target_link_libraries(P2P_server PRIVATE Threads::Threads)

add_executable(P2P_Client client_daemon.cpp client.cpp MessageParser.cpp)

# Link against pthread
target_link_libraries(P2P_Client PRIVATE Threads::Threads)