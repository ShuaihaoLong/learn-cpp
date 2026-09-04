#include <iostream>
#include <thread>
#include <string>
#include <memory>
#include <mutex>

std::mutex mx;
int a = 0;
void print_num(const int n) {
    for(int i = 0; i < n; i++) {
        std::lock_guard<std::mutex> lg(mx);
        a += 1;
        std::cout << a << std::endl;
    }
}

class A {
public:
    void foo(const std::string& msg) {
        std::cout << msg << std::endl;
    }
};

int main() {
    std::shared_ptr a1 = std::make_shared<A>(); 
    std::thread t1(print_num, 10);
    std::thread t2(print_num, 10);
    t1.join();
    t2.join();
    return 0;
}