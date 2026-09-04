#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

int counter = 0;

void worker() {
    for (int i = 0; i < 100000; ++i)
        ++counter;
}

std::mutex mx;

void workerWithMutex() {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mx);
        ++counter;
    }
}

std::atomic<int> couter_atomic = 0;
void workerWithAtomic() {
    for (int i = 0; i < 100000; ++i) {
        ++couter_atomic;
    }
}

int main() {
    std::cout << "--start--\n";
    std::thread t1(workerWithMutex);
    std::thread t2(workerWithMutex);
    t2.join();
    t1.join();
    std::cout << counter << "\n";
    std::thread t3(workerWithAtomic);
    std::thread t4(workerWithAtomic);
    t4.join();
    t3.join();
    std::cout << couter_atomic.load() << "\n";
    std::cout << "--end--\n";
    return 0;
}