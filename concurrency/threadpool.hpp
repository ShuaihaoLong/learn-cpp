#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <future>

class threadPool {
public:
    threadPool(std::size_t count) {
        if (count == 0) {
            throw std::invalid_argument("count must greater than 0");
        }
        for (size_t i = 0; i < count; i++) {
           workers_.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    cv_.wait(lock, [this]() {
                        return !tasks_.empty() || stop_;
                    });
                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
           }
           }); 
        }
    }
    ~threadPool() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    threadPool(const threadPool&) = delete;
    threadPool& operator=(const threadPool&) = delete;

    template <typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>> {
        using ReturnType = std::invoke_result_t<Func, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_) {
                throw std::runtime_error("submit on stopped ThreadPool");
            }

            tasks_.emplace([task] {
                (*task)();
            });
        }

        cv_.notify_one();
        return result;
    }

private:
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::vector<std::thread> workers_;
    bool stop_{false};
};