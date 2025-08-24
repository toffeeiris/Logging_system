#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

template<typename T>
class ThreadQueue
{
public:
    ThreadQueue() = default; 
    ~ThreadQueue() = default; 

    ThreadQueue(const ThreadQueue&) = delete;
    ThreadQueue& operator=(const ThreadQueue&) = delete;

    void push(T value)
    {
        std::lock_guard<std::mutex> lock(curr_mutex);
        curr_queue.push(std::move(value));
        curr_condition.notify_one();
    }

    bool pop(T& value)
    {
        std::lock_guard<std::mutex> lock(curr_mutex);
        if (curr_queue.empty())
            return false;

        value = std::move(curr_queue.front());
        curr_queue.pop();
        return true;
    }

    bool pop_with_wait(T& value)
    {
        std::unique_lock<std::mutex> lock(curr_mutex);
        curr_condition.wait(lock, [this] \
            {return !curr_queue.empty() || is_stop;});

        if (curr_queue.empty() && is_stop)
            return false;

        value = std::move(curr_queue.front());
        curr_queue.pop();
        return true;
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(curr_mutex);
        is_stop = true;
        curr_condition.notify_all();
    }

    bool empty() const 
    {
        std::lock_guard<std::mutex> lock(curr_mutex);
        return curr_queue.empty();
    }

    size_t size() const 
    {
        std::lock_guard<std::mutex> lock(curr_mutex);
        return curr_queue.size();
    }

    bool is_stopped() const 
    {
        return is_stop;
    }

private:
    mutable std::mutex curr_mutex;
    std::condition_variable curr_condition;
    std::queue<T> curr_queue;
    std::atomic<bool> is_stop = false;
};

#endif // THREAD_QUEUE_H