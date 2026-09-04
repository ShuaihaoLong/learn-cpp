#include <iostream>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

template <typename T>
class BlockingQueue {
public:
    BlockingQueue () : running_(true) {}
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        std::cout << "queue push " << value << "\n";
        cv_.notify_all();
    }
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] () {
            return !queue_.empty() || !running_;
        });
        if (queue_.empty()) {
            return std::nullopt;
        }
        T res = queue_.front();
        queue_.pop();
        std::cout << "queue pop " << res << "\n";
        return res;
    }
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        cv_.notify_all();
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_{false};
};

BlockingQueue<int> intBlockingQueue;

void producer() {
    int N = 10;
    while (N--) {
        intBlockingQueue.push(N);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    intBlockingQueue.stop();
}

void consumer() {
    int N = 10;
    while (N--) {
        intBlockingQueue.pop();
    }
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t2.join();
    t1.join();
    return 0;
}