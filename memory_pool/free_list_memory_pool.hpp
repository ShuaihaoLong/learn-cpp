#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <utility>
#include <vector>

class FreeListMemoryPool {
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

        std::byte* data() const {
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
        friend class FreeListMemoryPool;

        Handle(FreeListMemoryPool* pool, std::byte* ptr, std::size_t size)
            : pool_(pool), ptr_(ptr), size_(size) {}

        FreeListMemoryPool* pool_{nullptr};
        std::byte* ptr_{nullptr};
        std::size_t size_{0};
    };

    FreeListMemoryPool(std::size_t block_count, std::size_t block_size)
        : block_size_(block_size),
          storage_(block_count * block_size),
          in_use_(block_count, 0) {
        free_indices_.reserve(block_count);
        for (std::size_t i = 0; i < block_count; ++i) {
            free_indices_.push_back(block_count - 1 - i);
        }
    }

    FreeListMemoryPool(const FreeListMemoryPool&) = delete;
    FreeListMemoryPool& operator=(const FreeListMemoryPool&) = delete;
    FreeListMemoryPool(FreeListMemoryPool&&) = delete;
    FreeListMemoryPool& operator=(FreeListMemoryPool&&) = delete;

    Handle allocate(std::size_t size) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (size > block_size_ || free_indices_.empty()) {
            return Handle();
        }

        const std::size_t index = free_indices_.back();
        free_indices_.pop_back();
        in_use_[index] = 1;

        return Handle(this, block_ptr(index), size);
    }

    std::size_t capacity() const {
        return in_use_.size();
    }

    std::size_t block_size() const {
        return block_size_;
    }

    std::size_t available() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return free_indices_.size();
    }

private:
    void deallocate(std::byte* ptr) {
        if (ptr == nullptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(mtx_);

        const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(storage_.data());
        const std::uintptr_t current = reinterpret_cast<std::uintptr_t>(ptr);
        const std::uintptr_t end = base + storage_.size();

        if (current < base || current >= end) {
            return;
        }

        const std::size_t offset = static_cast<std::size_t>(current - base);
        if (block_size_ == 0 || offset % block_size_ != 0) {
            return;
        }

        const std::size_t index = offset / block_size_;
        if (index >= in_use_.size() || in_use_[index] == 0) {
            return;
        }

        in_use_[index] = 0;
        free_indices_.push_back(index);
    }

    std::byte* block_ptr(std::size_t index) {
        return storage_.data() + index * block_size_;
    }

    std::size_t block_size_{0};
    std::vector<std::byte> storage_;
    std::vector<std::size_t> free_indices_;
    std::vector<unsigned char> in_use_;
    mutable std::mutex mtx_;
};
