// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

// thread-safely encapsulates a deque, used as packet stream buffer
template <typename T> class Queue {
    private:
        std::mutex containerMutex;
        std::deque<T> container;
        std::condition_variable blockingCV;
        std::mutex blockingMutex;

    public:
        Queue() = default;
        Queue(const Queue<T>&) = delete;
        virtual ~Queue() { clear(); }

        // basic methods encapsulating an std::deque

        void wait() {
            while (empty()) {
                std::unique_lock<std::mutex> uniqueLock(blockingMutex);
                blockingCV.wait(uniqueLock);
            }
        }

        bool empty() {
            std::scoped_lock lock(containerMutex);
            return container.empty();
        }

        size_t count() {
            std::scoped_lock lock(containerMutex);
            return container.size();
        }

        void clear() {
            std::scoped_lock lock(containerMutex);
            container.clear();
        }

        const T& back() {
            std::scoped_lock lock(containerMutex);
            return container.back();
        }

        void push_back(const T& item) {
            std::scoped_lock lock(containerMutex);
            container.emplace_back(std::move(item));
            std::unique_lock<std::mutex> uniqueLock(blockingMutex);
            blockingCV.notify_one();
        }

        T pop_back() {
            std::scoped_lock lock(containerMutex);
            auto item = std::move(container.front());
            container.pop_back();
            return item;
        }

        const T& front() {
            std::scoped_lock lock(containerMutex);
            return container.front();
        }

        void push_front(const T& item) {
            std::scoped_lock lock(containerMutex);
            container.emplace_front(std::move(item));
            std::unique_lock<std::mutex> uniqueLock(blockingMutex);
            blockingCV.notify_one();
        }

        T pop_front() {
            std::scoped_lock lock(containerMutex);
            auto x = std::move(container.front());
            container.pop_front();
            return x;
        }
};