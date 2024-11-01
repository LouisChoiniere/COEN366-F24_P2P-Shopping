
#ifndef P2P_WITH_EVENTING_THREADPOOL_H
#define P2P_WITH_EVENTING_THREADPOOL_H

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


// Thread pool for handling concurrent peer connections
class ThreadPool {
public:
    explicit ThreadPool(size_t thread_count);
    template<typename F>
    auto enqueue(F &&f) -> std::future<typename std::result_of<F()>::type> {
        using return_type = typename std::result_of<F()>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("Cannot enqueue on stopped thread pool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }
    ~ThreadPool();

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

#endif //P2P_WITH_EVENTING_THREADPOOL_H
