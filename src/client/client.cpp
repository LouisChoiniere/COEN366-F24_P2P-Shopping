#include "client.h"

#include <fcntl.h>
#include <arpa/inet.h>

P2PClient::~P2PClient() {
    stop();
    close(client_socket_);
}

void P2PClient::start() {
    running_ = true;
    receive_thread_ = std::thread(&P2PClient::receiveMessages, this);
    startCommandLoop();
}

void P2PClient::stop() {
    running_ = false;
    if (receive_thread_.joinable()) { receive_thread_.join(); }
}

void P2PClient::startCommandLoop() {
    printHelp();

    std::string command;
    while (running_ && std::getline(std::cin, command)) {
        if (command.empty()) continue;

        try { processUserCommand(command); }
        catch (const std::exception& e) { std::cerr << "Error processing command: " << e.what() << std::endl; }
    }
}

void P2PClient::processUserCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "add") {
        std::string name, description;
        double price;

        iss >> name;

        iss.get(); // Remove leading space
        if (iss.peek() == '"') {
            iss.get(); // Skip the opening quote
            std::getline(iss, description, '"'); // Read until the closing quote
        }
        else { iss >> description; }

        iss >> price;

        addItem(name, description, price);
    }
    else if (cmd == "remove" | cmd == "rm") {
        std::string name;
        iss >> name;
        removeItem(name);
    }
    else if (cmd == "list" || cmd == "ls") { listInventory(); }
    else if (cmd == "help" || cmd == "h") { printHelp(); }
    else if (cmd == "register" || cmd == "r") { registerWithServer(); }
    else if (cmd == "deregister" || cmd == "d") { deregister(); }
    else if (cmd == "search" || cmd == "s") { handleSearchCommand(iss); }
    else if (cmd == "listOffers" || cmd == "ls offers" ) { handleListOffersCommand(iss); }
    else if (cmd == "negotiate" || cmd == "n") { handleNegotiateCommand(iss); }
    else if (cmd == "accept" || cmd == "a") { handleAcceptCommand(iss); }
    else if (cmd == "refuse" || cmd == "f") { handleRefuseCommand(iss); }
    else if (cmd == "status") { printStatus(); }
    else if (cmd == "quit" || cmd == "q") {
        if (current_state_ == P2PStateType::REGISTERED) { deregister(); }
        running_ = false;
    }
    else {
        std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
    }
}

void P2PClient::printHelp() {
    std::cout << "\n=== P2P Client Commands ===" << std::endl;
    std::cout << "register  (r) - Register with the server" << std::endl;
    std::cout << "deregister(d) - Deregister from the server" << std::endl;
    std::cout << "search    (s) <item_name> <description> <max_price> - Search for an item" << std::endl;
    std::cout << "listOffers(ls offers) - List active offers" << std::endl;
    std::cout << "negotiate (n) <request_number> <item_name> <counter_price> - Negotiate an offer" << std::endl;
    std::cout << "accept    (a) <request_number> <item_name> <price> - Accept an offer" << std::endl;
    std::cout << "refuse    (f) <request_number> <item_name> <price> - Refuse an offer" << std::endl;
    std::cout << "status        - Show current client status" << std::endl;
    std::cout << "add       <name> <description> <price> - Add item to inventory" << std::endl;
    std::cout << "remove    (rm) <name> - Remove item from inventory" << std::endl;
    std::cout << "list      (ls)- List all items in inventory" << std::endl;
    std::cout << "help      (h) - Show this help message" << std::endl;
    std::cout << "quit      (q) - Exit the client" << std::endl;
    std::cout << "=======================\n" << std::endl;
}

void P2PClient::printStatus() {
    std::cout << "\n=== Client Status ===" << std::endl;
    std::cout << "Name: " << name_ << std::endl;
    std::cout << "Server: " << server_ip_ << ":" << server_port_ << std::endl;
    std::cout << "Current State: " << getStateName(current_state_) << std::endl;
    std::cout << "Local Port: " << getLocalPort() << std::endl;
    std::cout << "==================\n" << std::endl;
}

std::string P2PClient::getStateName(P2PStateType state) {
    P2PState temp_state(state);
    return temp_state.getName();
}

void P2PClient::handleSearchCommand(std::istringstream& iss) {
    std::string item_name, description;
    double max_price;

    iss >> item_name;

    iss.get(); // Remove leading space
    if (iss.peek() == '"') {
        iss.get(); // Skip the opening quote
        std::getline(iss, description, '"'); // Read until the closing quote
    }
    else { iss >> description; }

    iss >> max_price;

    searchItem(item_name, description, max_price);
}

void P2PClient::handleListOffersCommand(std::istringstream& iss) {

    std::cout << "\n=== List of Offers ===" << std::endl;
    for (const auto& offer : offers_) {
        std::cout << "Request Number: " << offer.requestNumber << std::endl;
        std::cout << "Name" << offer.name << std::endl;
        std::cout << "Offered Price" << offer.price << std::endl;

        std::cout << std::endl;
    }
}

void P2PClient::handleNegotiateCommand(std::istringstream&) {
    int request_number;
    std::string item_name;
    double counter_price;

    std::cout << "Enter request number: ";
    std::cin >> request_number;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter item name: ";
    std::getline(std::cin, item_name);

    std::cout << "Enter counter offer price: ";
    std::cin >> counter_price;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    negotiateOffer(request_number, item_name, counter_price);
}

void P2PClient::handleAcceptCommand(std::istringstream&) {
    int request_number;
    std::string item_name;
    double price;

    std::cout << "Enter request number: ";
    std::cin >> request_number;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter item name: ";
    std::getline(std::cin, item_name);

    std::cout << "Enter price: ";
    std::cin >> price;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    acceptOffer(request_number, item_name, price);
}

void P2PClient::handleRefuseCommand(std::istringstream&) {
    int request_number;
    std::string item_name;
    double price;

    std::cout << "Enter request number: ";
    std::cin >> request_number;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter item name: ";
    std::getline(std::cin, item_name);

    std::cout << "Enter price: ";
    std::cin >> price;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    refuseOffer(request_number, item_name, price);
}

// Client operations
bool P2PClient::registerWithServer() {
    if (current_state_ != P2PStateType::UNREGISTERED) {
        std::cout << "Already registered or in process" << std::endl;
        return false;
    }

    json register_msg = {
        {"command", "REGISTER"},
        {"rq", getNextRequestNumber()},
        {"name", name_},
        {"ip", getLocalIpAddress()},
        {"udp_port", udp_port_},
        {"tcp_port", tcp_port_}
    };

    logOutgoingMessage(register_msg);
    if (sendMessage(register_msg)) {
        current_state_ = P2PStateType::REGISTERING;
        return true;
    }
    return false;
}

bool P2PClient::deregister() {
    if (current_state_ != P2PStateType::REGISTERED) {
        std::cout << "Not registered" << std::endl;
        return false;
    }

    json deregister_msg = {
        {"command", "DE_REGISTER"},
        {"rq", getNextRequestNumber()},
        {"name", name_}
    };

    logOutgoingMessage(deregister_msg);
    if (sendMessage(deregister_msg)) {
        current_state_ = P2PStateType::UNREGISTERED;
        return true;
    }
    return false;
}

bool P2PClient::searchItem(const std::string& item_name, const std::string& description, double max_price) {
    if (current_state_ != P2PStateType::REGISTERED) {
        std::cout << "Not registered with server" << std::endl;
        return false;
    }

    json search_msg = {
        {"command", "LOOKING_FOR"},
        {"rq", getNextRequestNumber()},
        {"name", name_},
        {"item_name", item_name},
        {"description", description},
        {"max_price", max_price}
    };

    logOutgoingMessage(search_msg);
    if (sendMessage(search_msg)) {
        current_state_ = P2PStateType::SEARCHING;
        return true;
    }
    return false;
}

bool P2PClient::negotiateOffer(int request_number, const std::string& item_name, double counter_price) {
    if (current_state_ != P2PStateType::SEARCHING && current_state_ != P2PStateType::NEGOTIATING) {
        std::cout << "Not in a valid state for negotiation" << std::endl;
        return false;
    }

    auto it = std::find_if(offers_.begin(), offers_.end(), [request_number](const Offer& offer) {
        return offer.requestNumber == request_number;
    });

    if (it == offers_.end()) {
        std::cout << "Offer not found for request number: " << request_number << std::endl;
        return false;
    }

    if (it.base()->name != item_name) {
        std::cout << "Item name does not match request number" << std::endl;
        return false;
    }

    offers_.erase(it);

    json negotiate_msg = {
        {"command", "NEGOTIATE"},
        {"rq", request_number},
        {"name", name_},
        {"item_name", item_name},
        {"price", counter_price}
    };

    logOutgoingMessage(negotiate_msg);
    if (sendMessage(negotiate_msg)) {
        current_state_ = P2PStateType::NEGOTIATING;
        return true;
    }
    return false;
}

bool P2PClient::acceptOffer(int request_number, const std::string& item_name, double price) {
    auto it = std::find_if(offers_.begin(), offers_.end(), [request_number](const Offer& offer) {
        return offer.requestNumber == request_number;
    });

    if (it == offers_.end()) {
        std::cout << "Offer not found for request number: " << request_number << std::endl;
        return false;
    }

    if (it.base()->name != item_name || it.base()->price != price) {
        std::cout << "Item name and price do not match" << std::endl;
        return false;
    }

    offers_.erase(it);

    json accept_msg = {
        {"command", "ACCEPT"},
        {"rq", request_number},
        {"name", name_},
        {"item_name", item_name},
        {"price", price}
    };

    logOutgoingMessage(accept_msg);
    return sendMessage(accept_msg);
}

bool P2PClient::refuseOffer(int request_number, const std::string& item_name, double price) {
    auto it = std::find_if(offers_.begin(), offers_.end(), [request_number](const Offer& offer) {
        return offer.requestNumber == request_number;
    });

    if (it == offers_.end()) {
        std::cout << "Offer not found for request number: " << request_number << std::endl;
        return false;
    }

    if (it.base()->name != item_name || it.base()->price != price) {
        std::cout << "Item name and price do not match" << std::endl;
        return false;
    }

    offers_.erase(it);

    json refuse_msg = {
        {"command", "REFUSE"},
        {"rq", request_number},
        {"name", name_},
        {"item_name", item_name},
        {"price", price}
    };

    logOutgoingMessage(refuse_msg);
    return sendMessage(refuse_msg);
}

void P2PClient::logOutgoingMessage(const json& msg) {
    std::cout << "\n=== Sending Message ===" << std::endl;
    MessageParser::printMessage(msg);
}

void P2PClient::setupSocket() {
    client_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket_ < 0) { throw std::runtime_error("Failed to create socket"); }

    // Bind to any available port
    sockaddr_in client_addr{};
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = udp_port_; // Let system assign port

    if (bind(client_socket_, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        close(client_socket_);
        throw std::runtime_error("Bind failed");
    }

    // Set socket to non-blocking mode
    int flags = fcntl(client_socket_, F_GETFL, 0);
    fcntl(client_socket_, F_SETFL, flags | O_NONBLOCK);
}

void P2PClient::receiveMessages() {
    char buffer[4096];
    sockaddr_in server_addr{};
    socklen_t server_len = sizeof(server_addr);

    while (running_) {
        ssize_t received = recvfrom(client_socket_, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr*)&server_addr, &server_len);

        if (received > 0) {
            buffer[received] = '\0';
            handleReceivedMessage(std::string(buffer));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void P2PClient::handleReceivedMessage(const std::string& message) {
    auto event = MessageParser::parseMessage(message);
    if (!event) {
        std::cerr << "Failed to parse message" << std::endl;
        return;
    }

    switch (event->getType()) {
    case P2PEventType::REGISTERED:
        current_state_ = P2PStateType::REGISTERED;
        std::cout << "Successfully registered with server" << std::endl;
        break;

    case P2PEventType::REGISTER_DENIED:
        current_state_ = P2PStateType::UNREGISTERED;
        std::cout << "Registration denied by server" << std::endl;
        break;

    case P2PEventType::SEARCH:
        handleSearchEvent(event);
        break;

    case P2PEventType::FOUND:
        current_state_ = P2PStateType::REGISTERED;
        std::cout << "Item found! Ready for purchase" << std::endl;
        break;

    case P2PEventType::NOT_FOUND:
        current_state_ = P2PStateType::REGISTERED;
        std::cout << "Item not found" << std::endl;
        break;

    case P2PEventType::OFFER:
    case P2PEventType::NEGOTIATE:
        handleOfferEvent(event);
        break;

    case P2PEventType::ACCEPT:
        // Remove item and write to console that it has been sold.
        break;

    case P2PEventType::REFUSE:
        std::cout << "Offer was refused for " << event->getData().item_name << "." << std::endl;

    default:
        std::cout << "Received message: " << message << std::endl;
        break;
    }
}

void P2PClient::handleOfferEvent(const std::shared_ptr<P2PEvent>& event) {
    if (current_state_ != P2PStateType::REGISTERED) {
        return; // Only registered peers can respond to searches
    }

    std::cout << "Received offer from peer (execute \"ls offers\" to see the offers)" << std::endl;
    auto messageData = event->getData();

    offers_.push_back({
        messageData.request_number,
        messageData.item_name,
        messageData.item_name,
        messageData.price,
    });
}

void P2PClient::handleSearchEvent(const std::shared_ptr<P2PEvent>& event) {
    if (current_state_ != P2PStateType::REGISTERED) {
        return; // Only registered peers can respond to searches
    }

    const auto& data = event->getData();
    std::string item_name = data.item_name;
    int request_number = data.request_number;

    // Check if we have the item in our inventory
    for (const auto& item : inventory_) {
        if (item.name == item_name) {
            // Found the item, send an offer
            json offer_msg = {
                {"command", "OFFER"},
                {"rq", request_number},
                {"name", name_}, // Our name as the offering peer
                {"item_name", item_name},
                {"price", item.price}
            };

            logOutgoingMessage(offer_msg);
            sendMessage(offer_msg);

            std::cout << "Sent offer for item: " << item_name
                << " at price: $" << item.price << std::endl;
            return;
        }
    }
}

// Buyer receives accept event
void P2PClient::handleAcceptEvent(const std::shared_ptr<P2PEvent>& event) {
    if (current_state_ != P2PStateType::REGISTERED) {
        return; // Only registered peers can respond to searches
    }

    const auto& data = event->getData();
    const std::string& name = data.item_name;

    // Buy Item
    json buy_msg = {
        {"command", "BUY"},
        {"rq", data.request_number},
        {"name", name_}, // Our name as the offering peer
        {"item_name", data.item_name},
        {"price", data.price},
    };

    logOutgoingMessage(buy_msg);
    sendMessage(buy_msg);
}

// Seller receives Buy event
void P2PClient::handleBuyEvent(const std::shared_ptr<P2PEvent>& event) {
    if (current_state_ != P2PStateType::REGISTERED) {
        return; // Only registered peers can respond to searches
    }

    const auto& data = event->getData();
    const std::string& name = data.item_name;

    // Check if item is still in inventory
    auto it = std::find_if(inventory_.begin(), inventory_.end(), [&name](const Item& item) {
        return item.name == name;
    });

    if (it == inventory_.end()) {
        std::cout << "Item was not found in inventory";
        return;
    }

    // Remove from inventory
    inventory_.erase(it);

    // Ship Item
    json buy_msg = {
        {"command", "SHIPPED"},
        {"rq", data.request_number},
        {"name", name_}, // Our name as the offering peer
        {"item_name", data.item_name}
    };

    std::cout << "Item " << name << " has been shipped to " << data.sender_name << std::endl;

    logOutgoingMessage(buy_msg);
    sendMessage(buy_msg);
}

void P2PClient::handleShippedEvent(const std::shared_ptr<P2PEvent>& event) {
    if (current_state_ != P2PStateType::REGISTERED) {
        return; // Only registered peers can respond to searches
    }


}




bool P2PClient::sendMessage(const json& msg) {
    std::string message = msg.dump();
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr);

    ssize_t sent = sendto(client_socket_, message.c_str(), message.length(), 0,
                          (struct sockaddr*)&server_addr, sizeof(server_addr));

    return sent == message.length();
}

int P2PClient::getNextRequestNumber() { return next_request_number_++; }

std::string P2PClient::getLocalIpAddress() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    struct hostent* host_entry = gethostbyname(hostname);
    if (!host_entry) { return "127.0.0.1"; }

    return inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
}

uint16_t P2PClient::getLocalPort() {
    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);
    getsockname(client_socket_, (struct sockaddr*)&addr, &addr_len);
    return ntohs(addr.sin_port);
}


struct Item {
    std::string name;
    std::string description;
    double price;
};

void P2PClient::addItem(const std::string& name, const std::string& description, double price) {
    inventory_.push_back({name, description, price});
    std::cout << "Added item to inventory: " << name << " at price: $" << price << std::endl;
}

void P2PClient::removeItem(const std::string& name) {
    auto it = std::find_if(inventory_.begin(), inventory_.end(),
                           [&name](const Item& item) { return item.name == name; });

    if (it != inventory_.end()) {
        inventory_.erase(it);
        std::cout << "Removed item from inventory: " << name << std::endl;
    }
}

void P2PClient::listInventory() {
    std::cout << "\n=== Current Inventory ===" << std::endl;
    if (inventory_.empty()) { std::cout << "No items in inventory" << std::endl; }
    else {
        for (const auto& item : inventory_) {
            std::cout << "Item: " << item.name << std::endl;
            std::cout << "Description: " << item.description << std::endl;
            std::cout << "Price: $" << std::fixed << std::setprecision(2) << item.price << std::endl;
            std::cout << "-------------------" << std::endl;
        }
    }
}
