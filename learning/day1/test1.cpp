/*2026.5.9
 *学习线程库的使用，如何用c++实现线程创建、回收
 */
#include <iostream>
#include <thread>
#include <chrono>

void print_num(int num) {
    while(num--) {
        std::cout << "thread" << std::this_thread::get_id() << ":" << num << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }  
}

int main() {
    std::thread th1(print_num, 10);
    std::thread th2(print_num, 10);

    th1.join();
    th2.join();
    return 0;
}