#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "Err_Manager.hpp"
#include "Channel.hpp"

class Acceptor;
class EventLoop
{
private:
    static EventLoop eventloop;

    std::unique_ptr<Channel> epfd;
    Acceptor *acceptor;
    std::vector<epoll_event> evs;
    std::unordered_map<int, std::weak_ptr<Channel>> channels;
    std::unique_ptr<Channel> _eventfd;
    int _errno;
    bool is_stop;

    std::mutex mtx;
    std::queue<std::function<void()>> tasks; // 工作线程提交的任务队列

    EventLoop();
    
public:
    static EventLoop &instance();
    bool init();
    
    void set_acceptor(Acceptor *&);
    
    void loop();
    
    int get_epfd() const;
    int get_eventfd() const;

    void update_Channel(Channel *&);
    
    void add_new_channel(const std::shared_ptr<Channel>&);
    void del_channel(int);
    
    std::function<void(int)> call_close_all;
    void set_call_close_all(std::function<void(int)>);

    template <typename Func, typename... Args>
    void enqueue(Func&& func, Args&&... args){
        std::unique_lock<std::mutex> lock(mtx);
        tasks.push([f = std::forward<Func>(func), ... args = std::forward<Args>(args)] { 
            f(args...); 
        });
    }

    EventLoop(EventLoop &&) = delete;
    EventLoop &operator=(EventLoop &&) = delete;
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    ~EventLoop() = default;
};