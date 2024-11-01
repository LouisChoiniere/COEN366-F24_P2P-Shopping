
#ifndef P2P_WITH_EVENTING_P2PEVENT_H
#define P2P_WITH_EVENTING_P2PEVENT_H

#include "Event.h"
#include <string>

// P2P specific events
enum class P2PEventType {
    REGISTER,
    REGISTER_DENIED,
    REGISTERED,
    DE_REGISTER,
    LOOKING_FOR,
    SEARCH,
    OFFER,
    NOT_AVAILABLE,
    NEGOTIATE,
    ACCEPT,
    FOUND,
    REFUSE,
    NOT_FOUND,
    RESERVE,
    CANCEL,
    BUY,
    UNKNOWN
};

class P2PEvent : public Event {
public:
    struct MessageData {
        int request_number;
        std::string sender_name;
        std::string ip_address;
        int udp_port;
        int tcp_port;
        std::string item_name;
        std::string item_description;
        double price;
        double max_price;
        std::string reason;

        MessageData(int rq, const std::string& name = "")
                : request_number(rq)
                , sender_name(name)
                , udp_port(0)
                , tcp_port(0)
                , price(0.0)
                , max_price(0.0) {}
    };

    P2PEvent(P2PEventType type, MessageData data)
            : type_(type), data_(data) {}

    P2PEventType getType() const { return type_; }
    const MessageData& getData() const { return data_; }

    std::string getName() const override {
        switch(type_) {
            case P2PEventType::REGISTER: return "REGISTER";
            case P2PEventType::REGISTER_DENIED: return "REGISTER-DENIED";
            case P2PEventType::REGISTERED: return "REGISTERED";
            case P2PEventType::DE_REGISTER: return "DE-REGISTER";
            case P2PEventType::LOOKING_FOR: return "LOOKING_FOR";
            case P2PEventType::SEARCH: return "SEARCH";
            case P2PEventType::OFFER: return "OFFER";
            case P2PEventType::NOT_AVAILABLE: return "NOT_AVAILABLE";
            case P2PEventType::NEGOTIATE: return "NEGOTIATE";
            case P2PEventType::ACCEPT: return "ACCEPT";
            case P2PEventType::FOUND: return "FOUND";
            case P2PEventType::REFUSE: return "REFUSE";
            case P2PEventType::NOT_FOUND: return "NOT_FOUND";
            case P2PEventType::RESERVE: return "RESERVE";
            case P2PEventType::CANCEL: return "CANCEL";
            case P2PEventType::BUY: return "BUY";
            default: return "UNKNOWN";
        }
    }

private:
    P2PEventType type_;
    MessageData data_;
};

#endif //P2P_WITH_EVENTING_P2PEVENT_H
