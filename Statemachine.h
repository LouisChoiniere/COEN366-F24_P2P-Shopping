

#ifndef P2P_WITH_EVENTING_STATEMACHINE_H
#define P2P_WITH_EVENTING_STATEMACHINE_H
// Generic state machine
template<typename StateType, typename EventType>
class StateMachine {
public:
    using StatePtr = std::shared_ptr<StateType>;
    using EventPtr = std::shared_ptr<EventType>;
    using Handler = std::function<StatePtr(const EventPtr&)>;
    using TransitionTable = std::unordered_map<
    std::string,
    std::unordered_map<std::string, Handler>
    >;

    StateMachine(StatePtr initial_state) : current_state_(initial_state) {}

    void addTransition(
            const StatePtr& from_state,
            const EventPtr& event,
            const Handler& handler
    ) {
        transitions_[from_state->getName()][event->getName()] = handler;
    }

    void processEvent(const EventPtr& event) {
        auto state_transitions = transitions_.find(current_state_->getName());
        if (state_transitions != transitions_.end()) {
            auto handler = state_transitions->second.find(event->getName());
            if (handler != state_transitions->second.end()) {
                auto new_state = handler->second(event);
                if (new_state) {
                    current_state_ = new_state;
                }
            }
        }
    }

    StatePtr getCurrentState() const { return current_state_; }

protected:
    StatePtr current_state_;
    TransitionTable transitions_;
};

#endif //P2P_WITH_EVENTING_STATEMACHINE_H
