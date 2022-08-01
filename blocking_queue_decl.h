#pragma once
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>

template <typename T>
class BlockingQueue : std::deque<T> {
    private:
        mutable std::mutex m;
        mutable std::condition_variable cv;
        std::deque<std::shared_ptr<T>> q;

    public:
        BlockingQueue();
        bool is_empty() const;
        void push(std::shared_ptr<T> value);
        std::shared_ptr<T> pop(std::atomic_bool& sig);
        void cv_notify_all();
};