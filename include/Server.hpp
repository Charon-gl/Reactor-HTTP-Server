#pragma once

#include <iostream>
#include <unordered_map>
#include <memory>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <exception>
#include "EventLoop.hpp"
#include "Channel.hpp"
#include "TCPConnection.hpp"

class Server
{
private:
    static Server server;

    uint16_t port;
    EventLoop *eventloop;
    Acceptor *acceptor;
    std::unordered_map<int, std::unique_ptr<TCPConnection>> clients;
    Server(uint16_t);

    void add_client(int);
    void del_client(int);

public:
    static Server &instance();
    void create();
    void run();
    
    Server(Server &&) = delete;
    Server(const Server &) = delete;
    Server &operator=(Server &&) = delete;
    Server &operator=(const Server &) = delete;

    ~Server();
};

