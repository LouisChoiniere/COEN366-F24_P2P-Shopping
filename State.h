
#ifndef P2P_WITH_EVENTING_STATE_H
#define P2P_WITH_EVENTING_STATE_H

// Generic state and event base classes
class State {
public:
    virtual ~State() = default;
    virtual std::string getName() const = 0;
};

#endif //P2P_WITH_EVENTING_STATE_H
