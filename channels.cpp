#include <cassert>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>
#include <condition_variable>
#include <variant>
#include <vector>

template<typename T>
class Channel {
		static constexpr std::size_t default_max_length = 30;
public:
		using value_type = T;

		explicit Channel(std::size_t max_length = default_max_length, std::size_t vector_size = 1)
				: max_length_(max_length), vector_size_(vector_size), use_vector_(vector_size > 1) {
						if (use_vector_) {
								buffer_ = std::vector<std::queue<value_type>>(vector_size_);
						}
				}
		// Copy constructor
		Channel(const Channel& other) {
			std::scoped_lock locker(other.mtx_);
			max_length_ = other.max_length_;
			vector_size_ = other.vector_size_;
			use_vector_ = other.use_vector_;
			closed_ = other.closed_;

			if (use_vector_) {
				const auto& other_queues = std::get<std::vector<std::queue<value_type>>>(other.buffer_);
				buffer_ = other_queues;  // Copy the contents
			} else {
				const auto& other_queue = std::get<std::queue<value_type>>(other.buffer_);
				buffer_ = other_queue;  // Copy the contents
			}
		}

		// Copy assignment operator
		Channel& operator=(const Channel& other) {
			if (this == &other) {
				return *this;  // Handle self-assignment
			}

			std::unique_lock locker(mtx_);
			std::unique_lock other_locker(other.mtx_);

			max_length_ = other.max_length_;
			vector_size_ = other.vector_size_;
			use_vector_ = other.use_vector_;
			closed_ = other.closed_;

			if (use_vector_) {
				const auto& other_queues = std::get<std::vector<std::queue<value_type>>>(other.buffer_);
				buffer_ = other_queues;  // Copy the contents
			} else {
				const auto& other_queue = std::get<std::queue<value_type>>(other.buffer_);
				buffer_ = other_queue;  // Copy the contents
			}

			return *this;
		}

		std::optional<value_type> next(int id) {
				std::unique_lock locker(mtx_);
				cond_.wait(locker, [this, id]() {
						return !empty(id) || closed_;
				});

				if (empty(id)) {
						assert(closed_);
						return std::nullopt;
				}

				value_type val = front(id);
				pop(id);
				cond_.notify_one();
				return val;
		}

		void put(int id, value_type val) {
				std::unique_lock locker(mtx_);
				cond_.wait(locker, [this, id]() {
						return size(id) < max_length_;
				});

				push(id, std::move(val));
				cond_.notify_one();
		}

		void close() {
				std::scoped_lock locker(mtx_);
				closed_ = true;
				cond_.notify_all();
		}

private:
		std::size_t max_length_;
		std::size_t vector_size_;
		bool use_vector_;
		std::variant<std::queue<value_type>, std::vector<std::queue<value_type>>> buffer_;
		bool closed_ = false;

		std::mutex mtx_;
		std::condition_variable cond_;

		// Helper functions to operate on the buffer variant
		bool empty(int id) const {
			if (use_vector_) {
				const auto& queues = std::get<std::vector<std::queue<value_type>>>(buffer_);
				return queues[id].empty();
			} else {
				return std::get<std::queue<value_type>>(buffer_).empty();
			}
		}

		std::size_t size(int id) const {
			if (use_vector_) {
				const auto& queues = std::get<std::vector<std::queue<value_type>>>(buffer_);
				return queues[id].size();
			} else {
				return std::get<std::queue<value_type>>(buffer_).size();
			}
		}

		value_type front(int id) const {
			if (use_vector_) {
				const auto& queues = std::get<std::vector<std::queue<value_type>>>(buffer_);
				return queues[id].front();
			} else {
				return std::get<std::queue<value_type>>(buffer_).front();
			}
		}

		void pop(int id) {
			if (use_vector_) {
				auto& queues = std::get<std::vector<std::queue<value_type>>>(buffer_);
				queues[id].pop();
			} else {
				std::get<std::queue<value_type>>(buffer_).pop();
			}
		}

		void push(int id, value_type val) {
			if (use_vector_) {
				auto& queues = std::get<std::vector<std::queue<value_type>>>(buffer_);
				queues[id].push(std::move(val));
			} else {
				std::get<std::queue<value_type>>(buffer_).push(std::move(val));
			}
		}
};