#pragma once

#include "../util/StateMachine.h"
#include "../P2P/P2PEvent.h"
#include "../P2P/P2PState.h"

// Peer implementation using generic state machine
class PeerStateMachine : public StateMachine<P2PState, P2PEvent> {
public:
    PeerStateMachine()
            : StateMachine(std::make_shared<P2PState>(P2PStateType::UNREGISTERED)) {
        setupTransitions();
    }

private:
    void setupTransitions() {
        addTransition(
                std::make_shared<P2PState>(P2PStateType::UNREGISTERED),
                std::make_shared<P2PEvent>(
                        P2PEventType::REGISTER,
                        P2PEvent::MessageData{0, ""}
                ),
                [](const std::shared_ptr<P2PEvent>& event) {
                    return std::make_shared<P2PState>(P2PStateType::REGISTERING);
                }
        );
    }
};
