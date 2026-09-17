#include <iostream>
#include <vector>

int main() {
    std::vector<int> arr(5);
    int i = 0;
    for (auto num : arr) {
        num = i;
        i++;
        std::cout << num << '\n';
    }

}