#include <iostream>
#include <thread>
#include <future>

std::future<int> asyncCalculate() {
    std::promise<int> p;
    std::future<int> f = p.get_future();

    std::thread([p = std::move(p)] () mutable {
        int result = 1 + 2 + 3;
        p.set_value(result);
    }).detach();
    return f;
}

int main() {
    auto f = asyncCalculate();
    int result = f.get();
    std::cout << result << '\n';
    return 0;
}