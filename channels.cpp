#include <cassert>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>
#include <condition_variable>

template<typename T>
class Channel {
    static constexpr std::size_t default_max_length = 30;
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
        
        value_type val = buffer_.top().second;
        buffer_.pop();
        cond_.notify_one();
        return val;
    }
    
    void put(int id, value_type val) {
        std::unique_lock locker(mtx_);
        cond_.wait(locker, [this]() {
            return buffer_.size() < max_length_;
        });

        buffer_.push(std::make_pair(id, std::move(val)));
        cond_.notify_one();
    }

    void close() {
        std::scoped_lock locker(mtx_);
        closed_ = true;
        cond_.notify_all();
    }

  private:
    std::size_t max_length_;  
    std::priority_queue<std::pair<int, value_type>, std::vector<std::pair<int, value_type>>, std::greater<>> buffer_;
    bool closed_ = false;    

    std::mutex mtx_;
    std::condition_variable cond_; 
};