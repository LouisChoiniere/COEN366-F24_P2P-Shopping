#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "ServerCommandHandlers.h"
#include "ServerStateMachine.h"
#include "../util/MessageParser.h"
#include "../util/ConcurrentQueue.h"
#include "../util/ThreadPool.h"


class ConcurrentServer {
public:
    ConcurrentServer(uint16_t port, size_t thread_count = std::thread::hardware_concurrency())
        : thread_pool_(thread_count),
          event_processor_thread_([this] { processEvents(); }),
          running_(true) {
        setupSocket(port);
        command_handlers_ = std::make_unique<ServerCommandHandlers>(
            server_socket_, peer_sessions_, sessions_mutex_);
    }

    void start() {
        while (running_) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            char buffer[1024];

            ssize_t received = recvfrom(server_socket_, buffer, sizeof(buffer) - 1, 0,
                                        (struct sockaddr*)&client_addr, &client_len);

            if (received > 0) {
                buffer[received] = '\0';
                handleNewMessage(std::string(buffer), client_addr);
            }
        }
    }

    void stop() {
        running_ = false;
        event_processor_thread_.join();
        close(server_socket_);
    }

    ~ConcurrentServer() {
        if (running_) {
            stop();
        }
    }

private:
    ThreadPool thread_pool_;
    std::thread event_processor_thread_;
    std::atomic<bool> running_;
    int server_socket_;
    std::unique_ptr<ServerCommandHandlers> command_handlers_;

    ConcurrentQueue<std::pair<std::shared_ptr<P2PEvent>, sockaddr_in>> event_queue_;
    std::unordered_map<std::string, std::shared_ptr<PeerSession>> peer_sessions_;
    std::mutex sessions_mutex_;
    ServerStateMachine server_state_machine_;

    void setupSocket(uint16_t port) {
        server_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_socket_ < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(server_socket_);
            throw std::runtime_error("Bind failed");
        }

        // Set socket to non-blocking mode
        int flags = fcntl(server_socket_, F_GETFL, 0);
        fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK);
    }

    void handleNewMessage(const std::string& message, const sockaddr_in& client_addr) {
        thread_pool_.enqueue([this, message, client_addr] {
            try {
                auto j = json::parse(message);
                std::cout << "\n=== Received Message ===" << std::endl;
                MessageParser::printMessage(j);

                command_handlers_->handleCommand(j, client_addr);

                auto event = parseMessage(message);
                if (event) {
                    event_queue_.push({event, client_addr});
                }
            }
            catch (const json::parse_error& e) {
                std::cerr << "Failed to parse message: " << e.what() << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing message: " << e.what() << std::endl;
            }
        });
    }

    void processEvents() {
        while (running_) {
            std::pair<std::shared_ptr<P2PEvent>, sockaddr_in> event_pair;
            event_queue_.wait_and_pop(event_pair);

            auto [event, client_addr] = event_pair;
            std::string peer_id = getPeerIdentifier(client_addr);

            std::lock_guard<std::mutex> lock(sessions_mutex_);
            auto session = peer_sessions_[peer_id];
            if (session) {
                // Process event in both server and peer state machines
                server_state_machine_.processEvent(event);
                session->processEvent(event);
            }
        }
    }

    std::shared_ptr<PeerSession> getOrCreateSession(
        const std::string& peer_id,
        const sockaddr_in& client_addr
    ) {
        auto it = peer_sessions_.find(peer_id);
        if (it == peer_sessions_.end()) {
            auto session = std::make_shared<PeerSession>(server_socket_, client_addr);
            peer_sessions_[peer_id] = session;
            return session;
        }
        return it->second;
    }

    std::string getPeerIdentifier(const sockaddr_in& addr) {
        return std::string(inet_ntoa(addr.sin_addr)) + ":" +
            std::to_string(ntohs(addr.sin_port));
    }

    std::shared_ptr<P2PEvent> parseMessage(const std::string& message) {
        return MessageParser::parseMessage(message);
    }

    void handleStateTransition(
        const std::shared_ptr<PeerSession>& session,
        const std::shared_ptr<P2PEvent>& event
    ) {
    }
};
