

#ifndef P2P_WITH_EVENTING_P2PSTATE_H
#define P2P_WITH_EVENTING_P2PSTATE_H

#include "State.h"

// P2P specific states
enum class P2PStateType {
    UNREGISTERED,
    REGISTERING,
    REGISTERED,
    SEARCHING,
    OFFERING,
    NEGOTIATING,
    BUYING,
    ERROR
};

class P2PState : public State {
public:
    explicit P2PState(P2PStateType type) : type_(type) {}
    P2PStateType getType() const { return type_; }

    std::string getName() const override {
        switch(type_) {
            case P2PStateType::UNREGISTERED: return "UNREGISTERED";
            case P2PStateType::REGISTERING: return "REGISTERING";
            case P2PStateType::REGISTERED: return "REGISTERED";
            case P2PStateType::SEARCHING: return "SEARCHING";
            case P2PStateType::OFFERING: return "OFFERING";
            case P2PStateType::NEGOTIATING: return "NEGOTIATING";
            case P2PStateType::BUYING: return "BUYING";
            case P2PStateType::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

private:
    P2PStateType type_;
};

#endif //P2P_WITH_EVENTING_P2PSTATE_H
