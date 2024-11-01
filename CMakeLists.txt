cmake_minimum_required(VERSION 3.25.1)
project(COEN366_F24_P2P_Shopping)

set(CMAKE_CXX_STANDARD 20)

# Add pthread library
find_package(Threads REQUIRED)

#target_link_libraries(P2P_server PRIVATE Threads::Threads)
#target_link_libraries(P2P_Client PRIVATE Threads::Threads)

# Add the source files
set(SOURCES
    main.cpp
    ConcurrentServer.h
    MessageParser.cpp
    ServerCommandHandlers.cpp
    ThreadPool.cpp
)

# Add the executable
add_executable(COEN366_F24_P2P_Shopping ${SOURCES})