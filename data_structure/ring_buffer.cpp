#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity)
        : cap_(capacity), buffer_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("capacity must be greater than zero");
        }
    }

    bool is_full() const {
        return count_ == cap_;
    }

    bool is_empty() const {
        return count_ == 0;
    }

    bool push(T value) {
        if (is_full()) {
            return false;
        }
        buffer_[tail_] = value;
        count_++;
        tail_ = (tail_ + 1) % cap_;
        return true;
    }

    bool pop(T& value) {
        if (is_empty()) {
            return false;
        }
        value = buffer_[head_];
        count_--;
        head_ = (head_ + 1) % cap_;
        return true;
    }

    void debug() const {
        if constexpr (std::is_same_v<T, int>) {
            for (std::size_t i = 0; i < count_; ++i) {
                std::cout << buffer_[(head_ + i) % cap_] << ' ';
            }
            std::cout << '\n';
        }
    }
private:
    std::size_t cap_;
    std::size_t head_{0};
    std::size_t tail_{0};
    std::size_t count_{0};
    std::vector<T> buffer_;
}; 

int main() {
    RingBuffer<int> rb(5);
    for (int i = 1; i < 6; i++) {
        rb.push(i);
        rb.debug();
    }
    for (int i = 1; i < 6; i++) {
        int value;
        rb.pop(value);
        std::cout << "pop: " << value << '\n';
        rb.debug();
    }
    return 0;
}
