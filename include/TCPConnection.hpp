#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <errno.h>
#include "Server.hpp"
#include "EventLoop.hpp"
#include "Channel.hpp"

class TCPConnection
{
private:
    std::shared_ptr<Channel> channel;
    EventLoop *eventloop;
    sockaddr_in addr;
    socklen_t *len;
    std::string recv_buf;
    std::string write_buf;
    bool set_write_listen;

public:
    TCPConnection(int fd, EventLoop* _eventloop);

    void handle_reading();
    void handle_writing();
    void disconnect();

    void pre_send(const std::string& data);     //将数据写入缓冲区，并注册写事件

    std::shared_ptr<Channel> get_channel() const;
    TCPConnection(TCPConnection &&) = delete;
    TCPConnection(const TCPConnection &) = delete;
    TCPConnection &operator=(TCPConnection &&) = delete;
    TCPConnection &operator=(const TCPConnection &) = delete;
    
    ~TCPConnection() = default;
};