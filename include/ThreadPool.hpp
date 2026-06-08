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
    
    template <typename Func, typename... Args>
    void add_task(Func&& func, Args&& ...args)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            task_queue.push([f = std::forward<Func>(func), ... args = std::forward<Args>(args)] { 
                f(args...); 
            });
        }
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