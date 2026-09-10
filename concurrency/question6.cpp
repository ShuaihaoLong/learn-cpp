#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

#include "threadpool.hpp"

std::mutex mtx;
void addTest (int a, int b) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << a*a + b*b << '\n';
}

int main() {
    threadPool thread_pool(3);
    thread_pool.submit(addTest, 100, 10);
    thread_pool.submit(addTest, 200, 10);
    thread_pool.submit(addTest, 300, 10);
    thread_pool.submit(addTest, 400, 10);
    std::this_thread::sleep_for(std::chrono::seconds(5));
}