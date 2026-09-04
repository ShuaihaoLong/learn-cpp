#include <iostream>
#include <future>

int func(int& i) {
    for(int k = 0; k < 1000; k++) {
        i++;
    }
    return i;
}

int main() {
    int cnt = 0;
    std::future<int> res = std::async(std::launch::async, func, std::ref(cnt));
    std::cout << res.get() << std::endl;

    return 0;
}