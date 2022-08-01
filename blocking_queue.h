#pragma once
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include "blocking_queue_decl.h"

template <typename T>
BlockingQueue<T>::BlockingQueue() : m{}, cv{}, q{} {};

template <typename T>
bool BlockingQueue<T>::is_empty() const {
    std::lock_guard<std::mutex> lock{m};
    return q.empty();
}

template <typename T>
void BlockingQueue<T>::push(std::shared_ptr<T> value) {
    {
        std::lock_guard<std::mutex> lock{m};
        q.push_back(value);
    }
    cv.notify_all();
}

template <typename T>
std::shared_ptr<T> BlockingQueue<T>::pop(std::atomic_bool& sig) {
    std::unique_lock<std::mutex> lock{m};
    while (q.empty()) {
        if (!sig) {
            return nullptr;
        }
        cv.wait(lock);
    }
    std::shared_ptr<T> element = std::move(q.front());
    q.pop_front();
    return element;
}

template <typename T>
void BlockingQueue<T>::cv_notify_all() {
    cv.notify_all();
}