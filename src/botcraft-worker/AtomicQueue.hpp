#pragma once
#include <mutex>
#include <queue>
#include <malloc.h>

#ifndef BOTCRAFT_WORKER_MAX_QUEUE
#define BOTCRAFT_WORKER_MAX_QUEUE 0
#endif

template<typename T>
class AtomicQueue {
    static constexpr int MAX_QUEUE_SIZE = BOTCRAFT_WORKER_MAX_QUEUE;
public:
    void push(const T &value) {
        if constexpr (MAX_QUEUE_SIZE > 0) if (size() >= MAX_QUEUE_SIZE) pop();
        std::lock_guard lock(mutex);
        T *v = new T();
        *v = value;
        queue.push(v);
    }

    void pop() {
        std::lock_guard lock(mutex);
        internal_pop();
    }

    T pull() {
        std::lock_guard lock(mutex);
        T v = *queue.front();
        internal_pop();
        return v;
    }

    T peek() const {
        std::lock_guard lock(mutex);
        return *queue.front();
    }

    int size() const {
        std::lock_guard lock(mutex);
        return queue.size();
    }

    ~AtomicQueue() {
        std::lock_guard lock(mutex);
        while (!queue.empty()) internal_pop();
    }

private:
    std::queue<T*> queue;
    mutable std::mutex mutex;

    void internal_pop() {
        if (queue.empty()) return;
        delete queue.front();
        queue.pop();
        if (queue.empty()) {
            std::queue<T*>().swap(queue);
            malloc_trim(0);
        }
    }
};
