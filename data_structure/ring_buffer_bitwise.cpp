#include <atomic>
#include <cstddef>
#include <iostream>
#include <new>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// Lock-free ring buffer for exactly one producer and one consumer.
// One storage slot is left empty to distinguish full from empty.
template <typename T>
class SpscRingBuffer {
private:
    static constexpr std::size_t cache_line_size = 64;
    static constexpr std::size_t storage_alignment =
        alignof(T) > cache_line_size ? alignof(T) : cache_line_size;
    using Storage =
        std::aligned_storage_t<sizeof(T), storage_alignment>;

    struct alignas(cache_line_size) ProducerState {
        std::atomic<std::size_t> tail{0};
        std::size_t cached_head{0};
    };

    struct alignas(cache_line_size) ConsumerState {
        std::atomic<std::size_t> head{0};
        std::size_t cached_tail{0};
    };

public:
    explicit SpscRingBuffer(std::size_t storage_capacity)
        : storage_capacity_(validate_capacity(storage_capacity)),
          mask_(storage_capacity_ - 1),
          buffer_(storage_capacity_) {}

    ~SpscRingBuffer() {
        std::size_t head = consumer_.head.load(std::memory_order_relaxed);
        const std::size_t tail =
            producer_.tail.load(std::memory_order_relaxed);

        while (head != tail) {
            element_at(head)->~T();
            head = (head + 1) & mask_;
        }
    }

    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

    template <typename U>
    bool push(U&& value) {
        return emplace(std::forward<U>(value));
    }

    template <typename... Args>
    bool emplace(Args&&... args) {
        const std::size_t tail = producer_.tail.load(std::memory_order_relaxed);
        const std::size_t next_tail = (tail + 1) & mask_;

        if (next_tail == producer_.cached_head) {
            producer_.cached_head =
                consumer_.head.load(std::memory_order_acquire);
            if (next_tail == producer_.cached_head) {
                return false;
            }
        }

        ::new (static_cast<void*>(&buffer_[tail]))
            T(std::forward<Args>(args)...);
        producer_.tail.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& value) {
        const std::size_t head = consumer_.head.load(std::memory_order_relaxed);

        if (head == consumer_.cached_tail) {
            consumer_.cached_tail =
                producer_.tail.load(std::memory_order_acquire);
            if (head == consumer_.cached_tail) {
                return false;
            }
        }

        T* const element = element_at(head);
        value = std::move(*element);
        element->~T();
        consumer_.head.store((head + 1) & mask_, std::memory_order_release);
        return true;
    }

    bool is_empty() const {
        return consumer_.head.load(std::memory_order_acquire) ==
               producer_.tail.load(std::memory_order_acquire);
    }

    bool is_full() const {
        const std::size_t tail =
            producer_.tail.load(std::memory_order_acquire);
        const std::size_t next_tail = (tail + 1) & mask_;
        return next_tail == consumer_.head.load(std::memory_order_acquire);
    }

    std::size_t capacity() const {
        return storage_capacity_ - 1;
    }

private:
    static std::size_t validate_capacity(std::size_t capacity) {
        if (capacity < 2 || (capacity & (capacity - 1)) != 0) {
            throw std::invalid_argument(
                "storage capacity must be a power of two and at least 2");
        }
        return capacity;
    }

    T* element_at(std::size_t index) {
        return std::launder(reinterpret_cast<T*>(&buffer_[index]));
    }

    const std::size_t storage_capacity_;
    const std::size_t mask_;
    std::vector<Storage> buffer_;

    // Separate ownership avoids false sharing between producer and consumer.
    ProducerState producer_;
    ConsumerState consumer_;
};

struct Item {
    Item() = delete;
    explicit Item(int value) : value(value) {}

    int value;
};

int main() {
    SpscRingBuffer<Item> buffer(1024);
    constexpr int item_count = 100000;
    std::atomic<bool> order_is_correct{true};

    std::thread producer([&buffer] {
        for (int value = 1; value <= item_count; ++value) {
            while (!buffer.emplace(value)) {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&buffer, &order_is_correct] {
        for (int expected = 1; expected <= item_count; ++expected) {
            Item item(0);
            while (!buffer.pop(item)) {
                std::this_thread::yield();
            }
            if (item.value != expected) {
                order_is_correct.store(false, std::memory_order_relaxed);
            }
        }
    });

    producer.join();
    consumer.join();

    std::cout << "usable capacity: " << buffer.capacity() << '\n';
    std::cout << "FIFO order: "
              << (order_is_correct.load(std::memory_order_relaxed) ? "correct"
                                                                  : "incorrect")
              << '\n';
    std::cout << "buffer empty: " << std::boolalpha << buffer.is_empty() << '\n';
    return 0;
}
