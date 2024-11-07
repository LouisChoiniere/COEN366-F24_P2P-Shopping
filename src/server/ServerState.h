#pragma once

#include <iostream>

#include "../util/State.h"

// Server implementation using generic state machine
enum class ServerStateType {
    LISTENING,
    PROCESSING_REGISTRATION,
    PROCESSING_SEARCH,
    PROCESSING_OFFER,
    PROCESSING_PURCHASE,
    ERROR
};

class ServerState : public State {
public:
    explicit ServerState(ServerStateType type) : type_(type) {
    }

    ServerStateType getType() const { return type_; }

    std::string getName() const override {
        switch (type_) {
        case ServerStateType::LISTENING: return "LISTENING";
        case ServerStateType::PROCESSING_REGISTRATION: return "PROCESSING_REGISTRATION";
        // Add other states...
        default: return "UNKNOWN";
        }
    }

private:
    ServerStateType type_;
};
