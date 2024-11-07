cmake_minimum_required(VERSION 3.25.1)
project(COEN366_F24_P2P_Shopping)

set(CMAKE_CXX_STANDARD 20)

# Add pthread library
find_package(Threads REQUIRED)

# Add the source files
file(GLOB_RECURSE SOURCES "src/*/*.cpp" "src/*/*.h")

# Add the executable
add_executable(Main src/main.cpp)
target_sources(Main PRIVATE
        src/main.cpp
        ${SOURCES}
)

add_executable(ClientExecutable src/client_daemon.cpp)
target_sources(ClientExecutable PRIVATE
        src/client_daemon.cpp
        ${SOURCES}
)

add_executable(ServerExecutable src/server_daemon.cpp)
target_sources(ServerExecutable PRIVATE
        src/server_daemon.cpp
        ${SOURCES}
)