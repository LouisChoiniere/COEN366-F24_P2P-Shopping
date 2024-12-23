#pragma once

#include <memory>

#include "ServerState.h"
#include "../util/StateMachine.h"
#include "../P2P/P2PEvent.h"

class ServerStateMachine : public StateMachine<ServerState, P2PEvent> {
public:
    ServerStateMachine()
        : StateMachine(std::make_shared<ServerState>(ServerStateType::LISTENING)) {
        setupTransitions();
    }

private:
    void setupTransitions() {
        // Listening -> Processing Registration
        addTransition(
            std::make_shared<ServerState>(ServerStateType::LISTENING),
            std::make_shared<P2PEvent>(
                P2PEventType::REGISTER,
                P2PEvent::MessageData{0, ""}
            ),
            [](const std::shared_ptr<P2PEvent>& event) {
                return std::make_shared<ServerState>(
                    ServerStateType::PROCESSING_REGISTRATION
                );
            }
        );

        // Add other transitions...
    }
};
