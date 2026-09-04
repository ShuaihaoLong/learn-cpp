#include <iostream>
#include <chrono>
#include <vector>
#include <future>
#include "MyThreadPool.hpp"

int slow_add(int lhs, int rhs) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return lhs + rhs;
}

int main() {
    ThreadPool pool(4);

    std::vector<std::future<int>> results;
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.submit(slow_add, i, i * 10));
    }

    pool.submit([] {
        std::cout << "hello from a worker thread" << std::endl;
    }).get();

    for (std::future<int>& result : results) {
        std::cout << "result: " << result.get() << std::endl;
    }

    return 0;
}