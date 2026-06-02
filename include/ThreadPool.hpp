#pragma once

#include <iostream>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <thread>
#include <functional>

class ThreadPool
{
public:
    ThreadPool(int num_threads);
    
    template <typename T, typename... Args>
    void add_task(T&& t, Args&& ...args)
    {
        task_queue.push([f = std::forward<T>(t), _args = std::forward<Args>(args)...] { 
            return std::function<void()>(std::forward<T>(f), std::forward<Args>(_args)...); 
        });
        condition.notify_one(); //添加了任务，通知一个线程来取任务
    }

    ~ThreadPool();

private:
    std::queue<std::function<void()>> task_queue;
    std::vector<std::thread> thread_pool;
    std::mutex mtx;
    std::condition_variable condition;
    bool is_stop;
};