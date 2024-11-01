
#ifndef P2P_WITH_EVENTING_PEERSESSION_H
#define P2P_WITH_EVENTING_PEERSESSION_H
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "PeerStateMachine.h"


class PeerSession {
public:
    PeerSession(int socket_fd, const sockaddr_in& peer_addr)
            : socket_fd_(socket_fd), peer_addr_(peer_addr), state_machine_() {}

    void processEvent(std::shared_ptr<P2PEvent> event) {
        state_machine_.processEvent(event);
    }

    const sockaddr_in& getPeerAddr() const { return peer_addr_; }
    int getSocketFd() const { return socket_fd_; }

private:
    int socket_fd_;
    sockaddr_in peer_addr_;
    PeerStateMachine state_machine_;
};


#endif //P2P_WITH_EVENTING_PEERSESSION_H
