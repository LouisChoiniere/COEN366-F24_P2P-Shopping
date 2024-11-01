
#ifndef P2P_WITH_EVENTING_EVENT_H
#define P2P_WITH_EVENTING_EVENT_H

class Event {
public:
    virtual ~Event() = default;
    virtual std::string getName() const = 0;
};

#endif //P2P_WITH_EVENTING_EVENT_H
