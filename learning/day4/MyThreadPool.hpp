#pragma once

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <memory>
#include <queue>
#include <vector>
#include <functional>
#include <stdexcept>
#include <type_traits>

class ThreadPool {
public:
    ThreadPool(std::size_t n) {
        if (n == 0) {
            throw std::invalid_argument("wrong size of pool!");
        }
        for(int i = 0; i < n; i++) {
            workers_.emplace_back([this] () {
                while(true) {

                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> ulock(mx_);
                        cv_.wait(ulock, [this] () {
                            return stop_ || !tasks_.empty();
                        });
                        if(stop_ && tasks_.empty()) {
                            return;
                        }
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    // 拿到函数后再执行，不耽误其它任务线程拿任务
                    task();
                }
            });
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> ulock(mx_);
            stop_ = true;
        }
        cv_.notify_all();
        for(std::thread& worker : workers_) {
            if(worker.joinable()) {
                worker.join();
            }
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    template<typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>> {
            using ReturnType = std::invoke_result_t<Func, Args...>;
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

            std::future<ReturnType> result = task->get_future();

            {
                std::lock_guard<std::mutex> glock(mx_);
                if(stop_) {
                    throw std::runtime_error("submit on stopped TreadPool");
                }
                tasks_.emplace([task] () {
                    (*task)();
                });
            }
            cv_.notify_one();
            return result;
        }
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mx_;
    std::condition_variable cv_;
    bool stop_{false};
};
