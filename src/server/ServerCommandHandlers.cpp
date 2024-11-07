#include "ServerCommandHandlers.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <arpa/inet.h>

#include "../util/MessageParser.h"

ServerCommandHandlers::ServerCommandHandlers(int socket,
                                             std::unordered_map<std::string, std::shared_ptr<PeerSession>> &peer_sessions,
                                             std::mutex &sessions_mutex)
        : server_socket_(socket),
          peer_sessions_(peer_sessions),
          sessions_mutex_(sessions_mutex) {
    registerHandlers();
}

// Add cleanup in destructor
ServerCommandHandlers::~ServerCommandHandlers() {
    // Clean up any active searches
    std::lock_guard<std::mutex> lock(searches_mutex_);
    active_searches_.clear();
}

void ServerCommandHandlers::handleCommand(const json &msg, const sockaddr_in &client_addr) {
    std::string command = msg["command"];
    auto it = command_handlers_.find(command);
    if (it != command_handlers_.end()) {
        it->second(msg, client_addr);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}

void ServerCommandHandlers::registerHandlers() {
    command_handlers_["REGISTER"] = [this](const json &msg, const sockaddr_in &client_addr) {
        handleRegister(msg, client_addr);
    };

    command_handlers_["DE_REGISTER"] = [this](const json &msg, const sockaddr_in &client_addr) {
        handleDeregister(msg, client_addr);
    };

    command_handlers_["LOOKING_FOR"] = [this](const json &msg, const sockaddr_in &client_addr) {
        handleLookingFor(msg, client_addr);
    };

    command_handlers_["OFFER"] = [this](const json &msg, const sockaddr_in &client_addr) {
        handleOffer(msg, client_addr);
    };
}

void ServerCommandHandlers::handleRegister(const json &msg, const sockaddr_in &client_addr) {
    std::string peer_name = msg["name"];
    std::string peer_id = getPeerIdentifier(client_addr);

    std::lock_guard<std::mutex> lock(sessions_mutex_);

    // Check if peer already exists
    if (peer_sessions_.find(peer_id) != peer_sessions_.end()) {
        json response = {
                {"command",        "REGISTER-DENIED"},
                {"request_number", msg["rq"]},
                {"reason",         "Peer already registered"}
        };
        sendToClient(response, client_addr);
        return;
    }

    // Create new peer session
    auto session = std::make_shared<PeerSession>(server_socket_, client_addr);
    peer_sessions_[peer_id] = session;

    // Send confirmation
    json response = {
            {"command", "REGISTERED"},
            {"rq",      msg["rq"]}
    };

    sendToClient(response, client_addr);
    std::cout << "Registered peer: " << peer_name << " at " << peer_id << std::endl;
}

void ServerCommandHandlers::handleDeregister(const json &msg, const sockaddr_in &client_addr) {
    std::string peer_id = getPeerIdentifier(client_addr);

    std::lock_guard<std::mutex> lock(sessions_mutex_);
    peer_sessions_.erase(peer_id);

    std::cout << "Deregistered peer: " << msg["name"] << std::endl;
}

void ServerCommandHandlers::handleLookingFor(const json& msg, const sockaddr_in& client_addr) {
    int request_number = msg["rq"];
    std::string item_name = msg["item_name"];
    double max_price = msg["max_price"];

    // Create new search request
    auto search = SearchRequest(
            request_number,
            msg["name"],
            item_name,
            max_price,
            client_addr
    );

    // Create timeout thread
    std::thread timeout_thread([this, request_number]() {
        // Wait for 1 minute
        std::this_thread::sleep_for(std::chrono::minutes(1));
        processOffersAfterTimeout(request_number);
    });
    timeout_thread.detach();

    // Store the search request
    {
        std::lock_guard<std::mutex> lock(searches_mutex_);
        active_searches_[request_number] = std::move(search);
    }

    // Broadcast search to all peers except searcher
    json search_broadcast = {
            {"command", "SEARCH"},
            {"rq", request_number},
            {"item_name", item_name},
            {"description", msg["description"]}
    };

    std::lock_guard<std::mutex> lock(sessions_mutex_);
    std::string searcher_id = getPeerIdentifier(client_addr);

    for (const auto& [peer_id, session] : peer_sessions_) {
        if (peer_id != searcher_id) {
            sendToClient(search_broadcast, session->getPeerAddr());
        }
    }
}


std::string ServerCommandHandlers::getPeerIdentifier(const sockaddr_in &addr) {
    return std::string(inet_ntoa(addr.sin_addr)) + ":" +
           std::to_string(ntohs(addr.sin_port));
}

void ServerCommandHandlers::sendToClient(const json &msg, const sockaddr_in &client_addr) {
    std::string message = msg.dump();
    sendto(server_socket_, message.c_str(), message.length(), 0,
           (struct sockaddr *) &client_addr, sizeof(client_addr));
}


void  ServerCommandHandlers::handleOffer(const json& msg, const sockaddr_in& client_addr) {
    int request_number = msg["rq"];
    std::string seller_name = msg["name"];
    std::string item_name = msg["item_name"];
    double offer_price = msg["price"];

    std::lock_guard<std::mutex> lock(searches_mutex_);
    auto search_it = active_searches_.find(request_number);

    if (search_it != active_searches_.end() && !search_it->second.offers_processed) {
        auto& search = search_it->second;
        auto now = std::chrono::steady_clock::now();

        // Check if within 1-minute window
        if (now - search.start_time < std::chrono::minutes(1)) {
            // Add offer to the list
            search.offers.emplace_back(seller_name, offer_price, client_addr);
            std::cout << "Received offer from " << seller_name
                      << " for request " << request_number
                      << " at price " << offer_price << std::endl;
        } else {
            std::cout << "Dropped late offer from " << seller_name
                      << " for request " << request_number << std::endl;
        }
    }
}

void  ServerCommandHandlers::processOffersAfterTimeout(int request_number) {
    std::lock_guard<std::mutex> lock(searches_mutex_);
    auto search_it = active_searches_.find(request_number);

    if (search_it != active_searches_.end() && !search_it->second.offers_processed) {
        auto& search = search_it->second;
        search.offers_processed = true;

        if (search.offers.empty()) {
            // No offers received
            json not_available_msg = {
                    {"command", "NOT_AVAILABLE"},
                    {"rq", search.request_number},
                    {"item_name", search.item_name},
                    {"price", search.max_price}
            };
            sendToClient(not_available_msg, search.searcher_addr);
            active_searches_.erase(search_it);
            return;
        }

        // Find lowest price offer
        auto lowest_offer = std::min_element(
                search.offers.begin(),
                search.offers.end(),
                [](const OfferInfo& a, const OfferInfo& b) {
                    return a.price < b.price;
                }
        );

        if (lowest_offer->price <= search.max_price) {
            // Found an acceptable offer, notify buyer
            json found_msg = {
                    {"command", "FOUND"},
                    {"rq", search.request_number},
                    {"item_name", search.item_name},
                    {"price", lowest_offer->price}
            };
            sendToClient(found_msg, search.searcher_addr);
        } else {
            // Best offer is above max price, try negotiation
            json negotiate_msg = {
                    {"command", "NEGOTIATE"},
                    {"rq", search.request_number},
                    {"item_name", search.item_name},
                    {"max_price", search.max_price}
            };
            sendToClient(negotiate_msg, lowest_offer->seller_addr);
        }
    }
}