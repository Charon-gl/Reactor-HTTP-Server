#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include "Acceptor.hpp"
#include "Channel.hpp"

class EventLoop
{
private:
    static EventLoop eventloop;

    int epfd;
    Acceptor *acceptor;
    std::vector<epoll_event> evs;
    std::unordered_map<int, std::shared_ptr<Channel>> channels;

    EventLoop();
    void init();

public:
    static EventLoop &instance();

    void set_acceptor(Acceptor *);

    void loop();

    int get_epfd() const;

    void update_Channel(Channel *);

    void add_new_channel(Channel *);
    void del_channel(int);

    EventLoop(EventLoop &&) = delete;
    EventLoop &operator=(EventLoop &&) = delete;
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    ~EventLoop() = default;
};