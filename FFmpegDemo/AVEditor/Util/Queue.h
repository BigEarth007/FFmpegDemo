#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace aveditor
{
	template<typename T>
	class AVEDITOR_API Queue {
	public:
		Queue() {}
		~Queue() {}

		void Push(const T& element) {
			std::unique_lock<std::mutex> lock(_mutex);
			_queue.push(element);
			//_empty_notify.notify_all();
		}

		int Pop(T& val, const int n_nTimeout) {
			std::unique_lock<std::mutex> lock(_mutex);

			if (_queue.empty()) {
				//_empty_notify.wait(_mutex, [this]() {return !this->_queue.empty(); });
				//if (_empty_notify.wait_for(_mutex, std::chrono::milliseconds(n_nTimeout), 
				//	[this]() {return this->_queue.empty(); }))
				//	return -1;
				return -1;
			}

			val = std::move(_queue.front());
			_queue.pop();

			return 0;
		}

		int Front(T& val) {
			std::unique_lock<std::mutex> lock(_mutex);

			if (_queue.empty()) return -1;

			val = _queue.front();

			return 0;
		}

		void Clear(std::function<void(T&)> func = nullptr) {
			std::unique_lock<std::mutex> lock(_mutex);
			while (!_queue.empty()) {
				T val = std::move(_queue.front());
				_queue.pop();
				if (func) func(val);
			}
		}

		size_t Size() {
			std::unique_lock<std::mutex> lock(_mutex);
			return _queue.size();
		}

		bool Empty() {
			std::unique_lock<std::mutex> lock(_mutex);
			return _queue.empty();
		}

	private:
		std::mutex    _mutex;
		std::queue<T> _queue;
		//std::condition_variable_any _empty_notify;
	};
}