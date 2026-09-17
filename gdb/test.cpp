#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::mutex mtx1;
std::mutex mtx2;

void task1() {
    std::lock_guard<std::mutex> lock1(mtx1);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));

    std::lock_guard<std::mutex> lock2(mtx2);

    std::cout << "task1 finished\n";
}

void task2() {
    std::lock_guard<std::mutex> lock2(mtx2);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));

    std::lock_guard<std::mutex> lock1(mtx1);

    std::cout << "task2 finished\n";
}

int main() {
    std::thread t1(task1);
    std::thread t2(task2);

    t1.join();
    t2.join();
}