#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
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
    std::unordered_map<int, std::shared_ptr<Channel>> channels;

    bool is_stop;
    int _errno;

    EventLoop();
    
public:
    static EventLoop &instance();
    bool init();
    
    void set_acceptor(Acceptor *&);
    
    void loop();
    
    int get_epfd() const;
    
    void update_Channel(Channel *&);
    
    void add_new_channel(const std::shared_ptr<Channel>&);
    void del_channel(int);
    
    std::function<void(int)> call_close_all;
    void set_call_close_all(std::function<void(int)>);

    void run_in_loop(std::function<void(int)>, int);

    EventLoop(EventLoop &&) = delete;
    EventLoop &operator=(EventLoop &&) = delete;
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    ~EventLoop() = default;
};