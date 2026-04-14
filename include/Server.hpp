#pragma once

#include <iostream>
#include <unordered_set>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

class Server
{
private:
    int lfd;
    int epfd;
    sockaddr_in addr;
    std::unordered_set<int> cfds;

    void init();
    void run();

public:
    Server();
    Server(const Server &) = delete;
    Server(Server &&) = delete;
    ~Server();
};

