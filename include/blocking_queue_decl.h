#pragma once
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

// This BlockQueue wraps a standard deque,
// by having an additional member mutex and condition_variable
// As a result, it is thread safe
// So, a thread will block if another thread is currently pushing data
// into the queue, for example
template <typename T>
class BlockingQueue {
    private:
        mutable std::mutex m;
        mutable std::condition_variable cv;
        std::deque<T> q;

    public:
        using value_type = T;
        BlockingQueue();
        bool is_empty() const;
        void push(T value);
        // sig tells us if the pipeline has been stopped or not
        // If sig is false, the pipeline has been stopped
        // pop and peek return std::optionals to indicate that whether the pipeline
        // was stopped while these operations were being performed
        // A Component can then check if the returned std::optional has_value(),
        // to then return and stop itself (since a Component runs on its separate thread)
        std::optional<T> pop(std::atomic_bool& sig);
        std::optional<T> peek(std::atomic_bool& sig);
        void cv_notify_all();
};