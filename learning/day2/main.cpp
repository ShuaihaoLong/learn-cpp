//2026.5.17 练习线程库
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <future>

int test(int n) {
    int sum = 0;
    while(n--) {
        sum++;
        std::cout << "value = " << sum << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return sum;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cerr << "wrong usage of " << argv[0] << '\n';
        exit(1);
    }
    auto start_time = std::chrono::system_clock::now();
    std::cout << "--main function starts (" << std::chrono::system_clock::to_time_t(start_time) << ") --\n";

    int input = std::stoi(argv[1]); 
    std::cout << "input: " << input << '\n';
    std::future output_f = std::async(std::launch::async, test, input);
    int output = output_f.get(); 
    std::cout << "output: " << output << '\n';

    auto end_time = std::chrono::system_clock::now();
    auto dur_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);   
    std::cout << "--main function ends (" << std::chrono::system_clock::to_time_t(end_time) << ") --\n";
    std::cout << "takes time for " << dur_time.count() << "ms\n";
    exit(0);
}