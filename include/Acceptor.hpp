#pragma once

#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <functional>
#include <cstring>
#include <memory>
#include "Channel.hpp"
#include "Err_Manager.hpp"

class EventLoop;
class Acceptor
{
private:
    std::shared_ptr<Channel> lfd;
    sockaddr_in addr;
    EventLoop *eventloop;
    Acceptor(EventLoop*&);
    std::function<void(int)> add_client_callback;

public:
    static Acceptor& instance(EventLoop*&);

    int init_listen_fd(uint16_t);

    std::shared_ptr<Channel> get_lfd() const;
    int accept_fd();
    void set_add_client(std::function<void(int)>);

    Acceptor(Acceptor &&) = delete;
    Acceptor &operator=(Acceptor &&) = delete;
    Acceptor(const Acceptor &) = delete;
    Acceptor &operator=(const Acceptor &) = delete;

    ~Acceptor() = default;
};