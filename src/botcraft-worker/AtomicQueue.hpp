#pragma once
#include <mutex>
#include <queue>
#include <malloc.h>

template<typename T>
class AtomicQueue {
public:
    void push(const T &value) {
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

private:
    std::queue<T*> queue;
    mutable std::mutex mutex;

    void internal_pop() {
        delete queue.front();
        queue.pop();
        if (queue.empty()) {
            std::queue<T*>().swap(queue);
            malloc_trim(0);
        }
    }
};
