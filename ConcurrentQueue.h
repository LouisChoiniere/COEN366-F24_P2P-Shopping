
#ifndef P2P_WITH_EVENTING_CONCURRENTQUEUE_H
#define P2P_WITH_EVENTING_CONCURRENTQUEUE_H

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


template<typename T>
class ConcurrentQueue {
public:
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        lock.unlock();
        cond_.notify_one();
    }

    void wait_and_pop(T &item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]() { return !queue_.empty(); });
        item = std::move(queue_.front());
        queue_.pop();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif //P2P_WITH_EVENTING_CONCURRENTQUEUE_H
