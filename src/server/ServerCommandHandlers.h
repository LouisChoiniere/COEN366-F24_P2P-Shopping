#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "PeerSession.h"
#include "../util/MessageParser.h"

class ServerCommandHandlers {
public:
    ServerCommandHandlers(int socket,
                          std::unordered_map<std::string, std::shared_ptr<PeerSession>>& peer_sessions,
                          std::mutex& sessions_mutex);

    ~ServerCommandHandlers();

    void handleCommand(const json& msg, const sockaddr_in& client_addr);

private:
    using CommandHandler = std::function<void(const json&, const sockaddr_in&)>;

    int server_socket_;
    std::unordered_map<std::string, CommandHandler> command_handlers_;
    std::unordered_map<std::string, std::shared_ptr<PeerSession>>& peer_sessions_;
    std::mutex& sessions_mutex_;

    struct OfferInfo {
        std::string seller_name;
        double price;
        sockaddr_in seller_addr;

        // Default constructor
        OfferInfo() : seller_name(""), price(0.0), seller_addr{} {
        }

        OfferInfo(std::string name, double p, sockaddr_in addr)
            : seller_name(std::move(name))
              , price(p)
              , seller_addr(addr) {
        }
    };

    struct SearchRequest {
        int request_number;
        std::string searcher_name;
        std::string item_name;
        double max_price;
        sockaddr_in searcher_addr;
        std::chrono::steady_clock::time_point start_time;
        std::vector<OfferInfo> offers;
        bool offers_processed;

        // Default constructor
        SearchRequest()
            : request_number(0)
              , searcher_name("")
              , item_name("")
              , max_price(0.0)
              , searcher_addr{}
              , start_time(std::chrono::steady_clock::now())
              , offers_processed(false) {
        }

        SearchRequest(int rq, std::string name, std::string item, double price, sockaddr_in addr)
            : request_number(rq)
              , searcher_name(std::move(name))
              , item_name(std::move(item))
              , max_price(price)
              , searcher_addr(addr)
              , start_time(std::chrono::steady_clock::now())
              , offers_processed(false) {
        }
    };

    std::unordered_map<int, SearchRequest> active_searches_;
    std::mutex searches_mutex_;

    void registerHandlers();
    void handleRegister(const json& msg, const sockaddr_in& client_addr);
    void handleDeregister(const json& msg, const sockaddr_in& client_addr);
    void handleLookingFor(const json& msg, const sockaddr_in& client_addr);
    void handleOffer(const json& msg, const sockaddr_in& client_addr);
    std::string getPeerIdentifier(const sockaddr_in& addr);
    void sendToClient(const json& msg, const sockaddr_in& client_addr);
    void processOffersAfterTimeout(int request_number);
};
