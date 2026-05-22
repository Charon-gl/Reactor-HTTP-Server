#pragma once

#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <functional>
#include <cstring>
#include "EventLoop.hpp"
#include "TCPConnection.hpp"

class Acceptor
{
private:
    int lfd;
    sockaddr_in addr;
    EventLoop *eventloop;
    Acceptor(uint16_t, EventLoop*);
    std::function<void(int)> add_client_callback;

public:
    static Acceptor& instance(uint16_t, EventLoop*);

    int get_lfd() const;
    void accept_fd();
    void set_add_client(std::function<void(int)>);

    Acceptor(Acceptor &&) = delete;
    Acceptor &operator=(Acceptor &&) = delete;
    Acceptor(const Acceptor &) = delete;
    Acceptor &operator=(const Acceptor &) = delete;

    ~Acceptor();
};