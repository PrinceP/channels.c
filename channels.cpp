#include <cassert>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>
#include <condition_variable>

template<typename T>
class Channel {
    static constexpr std::size_t default_max_length = 10;
  public:
    using value_type = T;

    explicit Channel(std::size_t max_length = default_max_length)
      : max_length_(max_length) {}

    std::optional<value_type> next() {
        std::unique_lock locker(mtx_);
        cond_.wait(locker, [this]() {
            return !buffer_.empty() || closed_; 
        });
        
        if (buffer_.empty()) {
          assert(closed_);
          return std::nullopt;
        }
        
        value_type val = buffer_.front();
        buffer_.pop();
        cond_.notify_one();
        return val;
    }
    
    void put(value_type val) {
        std::unique_lock locker(mtx_);
        cond_.wait(locker, [this]() {
            return buffer_.size() < max_length_;
        });

        buffer_.push(std::move(val));
        cond_.notify_one();
    }

    void close() {
        std::scoped_lock locker(mtx_);
        closed_ = true;
        cond_.notify_all();
    }

  private:
    std::size_t max_length_;  
    std::queue<value_type> buffer_;
    bool closed_ = false;    

    std::mutex mtx_;
    std::condition_variable cond_; 
};

