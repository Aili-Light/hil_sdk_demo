#pragma once

#include <deque>

template<typename T>
class RingBuffer {
public:
    RingBuffer(size_t size) : maxSize(size) {}

    void push_back(const T& value) {
        if (buffer.size() >= maxSize) {
            buffer.pop_front();
        }
        buffer.push_back(value);

        if (buffer.size() == maxSize && full_callback) {
            full_callback();
        }
    }

    T pop_front() {
        if (buffer.empty()) {
            return nullptr;
        }

        T val = buffer.front();
        buffer.pop_front();
        if (buffer.empty() && empty_callback) empty_callback();
        return val;
    }

    T front() const {
        return buffer.front();
    }

    size_t size() const {
        return buffer.size();
    }

    bool empty() const {
        return buffer.empty();
    }

    void set_full_callback(std::function<void()> callback) {
        full_callback = callback;
    }
    
    void set_empty_callback(std::function<void()> callback) {
        empty_callback = callback;
    }

    void resize(size_t new_size) {
        maxSize = new_size;
        while (buffer.size() > maxSize) {
            buffer.pop_front();
        }
    }

private:
    std::deque<T> buffer;
    size_t maxSize;
    std::function<void()> full_callback;
    std::function<void()> empty_callback;
};

