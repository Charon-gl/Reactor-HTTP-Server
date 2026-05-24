#pragma once

#include <iostream>
#include <unordered_map>
#include <memory>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include "EventLoop.hpp"
#include "Acceptor.hpp"
#include "TCPConnection.hpp"
#include "Logger.hpp"
#include "err_to_string.hpp"

class Server
{
private:
    EventLoop *eventloop;
    uint16_t port;
    Acceptor *acceptor;
    std::unordered_map<int, std::unique_ptr<TCPConnection>> clients;
    
    Server();

    void set_port(u_int16_t);
    bool create();

    void add_client(int);
    void del_client(int, int);
    void del_all(int);

public:
    static Server &instance();
    bool run(uint16_t);
    
    Server(Server &&) = delete;
    Server(const Server &) = delete;
    Server &operator=(Server &&) = delete;
    Server &operator=(const Server &) = delete;

    ~Server() = default;
};

