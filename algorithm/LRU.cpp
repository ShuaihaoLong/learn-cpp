#include <cstddef>
#include <iostream>
#include <list>
#include <mutex>
#include <unordered_map>

template<typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(std::size_t capacity) : cap_(capacity) {}

    bool get(const K& key, V& value) {
       std::lock_guard<std::mutex> lock(mtx_);
       auto it = map_.find(key);
       if (it == map_.end()) {
            return false;
       }
       else {
        cache_.splice(cache_.begin(), cache_, it->second);
        value = it->second->second;
       }
       return true;
    } 

    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (cap_ == 0) {
            return;
        }

        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = value;
            cache_.splice(cache_.begin(), cache_, it->second);
            return;
        }
        if (cache_.size() >= cap_) {
            K old_key = cache_.back().first;
            map_.erase(old_key);
            cache_.pop_back();
        }
        cache_.emplace_front(key, value);
        map_[key] = cache_.begin();
    }

    void debug() {
        for (auto node : cache_) {
            std::cout << '[' << node.first << ']' << node.second << ' '; 
        } 
    }
private:
    std::list<std::pair<K, V>> cache_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> map_;
    std::size_t cap_;
    std::mutex mtx_;
};

int main() {
    LRUCache<int, std::string> cache(3);
    cache.put(1, "hello");
    cache.put(2, "cpp");
    cache.put(3, "linux");
    cache.debug();
    std::string str;
    cache.get(1, str);
    cache.get(2, str);
    cache.debug();

    return 0;
}
