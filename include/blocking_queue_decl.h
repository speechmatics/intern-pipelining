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
        std::deque<T> q;

    public:
        using value_type = T;
        BlockingQueue();
        bool is_empty() const;
        void push(T value);
        T pop(std::atomic_bool& sig);
        void cv_notify_all();
};