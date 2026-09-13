#pragma once

#include <cstddef>
#include <mutex>
#include <utility>
#include <vector>

class SimpleMemoryPool {
public:
    class Handle {
    public:
        Handle() = default;

        Handle(const Handle&) = delete;
        Handle& operator=(const Handle&) = delete;

        Handle(Handle&& other) noexcept
            : pool_(std::exchange(other.pool_, nullptr)),
              ptr_(std::exchange(other.ptr_, nullptr)),
              size_(std::exchange(other.size_, 0)) {}

        Handle& operator=(Handle&& other) noexcept {
            if (this != &other) {
                release();
                pool_ = std::exchange(other.pool_, nullptr);
                ptr_ = std::exchange(other.ptr_, nullptr);
                size_ = std::exchange(other.size_, 0);
            }
            return *this;
        }

        ~Handle() {
            release();
        }

        std::byte* get() const {
            return ptr_;
        }

        std::size_t size() const {
            return size_;
        }

        explicit operator bool() const {
            return ptr_ != nullptr;
        }

        void release() {
            if (pool_ != nullptr && ptr_ != nullptr) {
                pool_->deallocate(ptr_);
                pool_ = nullptr;
                ptr_ = nullptr;
                size_ = 0;
            }
        }

    private:
        friend class SimpleMemoryPool;

        Handle(SimpleMemoryPool* pool, std::byte* ptr, std::size_t size)
            : pool_(pool), ptr_(ptr), size_(size) {}

        SimpleMemoryPool* pool_{nullptr};
        std::byte* ptr_{nullptr};
        std::size_t size_{0};
    };

    SimpleMemoryPool(std::size_t block_size, std::size_t buffer_size) {
        blocks_.reserve(block_size);
        for (std::size_t i = 0; i < block_size; i++) {
            blocks_.push_back(Block{std::vector<std::byte>(buffer_size), false});
        }
    }

    SimpleMemoryPool(const SimpleMemoryPool&) = delete;
    SimpleMemoryPool& operator=(const SimpleMemoryPool&) = delete;

    Handle allocate(std::size_t size) {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& block : blocks_) {
            if (block.buffer.size() >= size && !block.used) {
                block.used = true;
                return Handle(this, block.buffer.data(), size);
            }
        }
        return Handle();
    }

private:
    void deallocate(std::byte* ptr) {
        if (ptr == nullptr) return;
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& block : blocks_) {
            if (block.buffer.data() == ptr) {
                block.used = false;
                return;
            }
        }
    }

    struct Block {
        std::vector<std::byte> buffer;
        bool used{false};
    };
    std::vector<Block> blocks_;
    std::mutex mtx_;
};
