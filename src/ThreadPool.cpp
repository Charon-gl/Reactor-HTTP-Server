#include "ThreadPool.hpp"

ThreadPool::ThreadPool(int num_threads) : thread_pool(std::vector<std::thread>(num_threads)), is_stop(false)
{
    for (int i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back(std::thread([this] {
            while(1)
            {
                std::unique_lock<std::mutex> lock(mtx);
                condition.wait(lock, [this]{    //无任务且未收到停止信号则阻塞
                    return !task_queue.empty() || is_stop; 
                });

                if(is_stop && task_queue.empty())   //收到结束信号且无任务则返回
                    return;

                std::function<void()> task = std::move(task_queue.front());
                task_queue.pop();

                //拿到任务，释放锁
                lock.unlock();
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        is_stop = true;
        condition.notify_all();
    }
    for(auto &i : thread_pool)
        i.join();
}