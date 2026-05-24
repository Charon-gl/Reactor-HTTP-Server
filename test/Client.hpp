#pragma once

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

class Client
{
private:
    int fd;
    sockaddr_in addr;
    int host_addr;
    int host_port;

public:
    Client(int _host_port);

    void run();

    ~Client();
};