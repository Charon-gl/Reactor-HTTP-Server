#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <errno.h>
#include <functional>
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
    int write_buf_len;
    int pre_pos;
    bool set_write_listen;

    std::function<void(int, int)> disconnect_callback; // 绑定的是Server的del_client()

public:
    TCPConnection(int fd, EventLoop* _eventloop);

    int handle_reading();
    int handle_writing();

    void set_disconnect(std::function<void(int, int)>);

    void pre_send(const std::string& data);     //将数据写入缓冲区，并注册写事件

    std::shared_ptr<Channel> get_channel() const;
    TCPConnection(TCPConnection &&) = delete;
    TCPConnection(const TCPConnection &) = delete;
    TCPConnection &operator=(TCPConnection &&) = delete;
    TCPConnection &operator=(const TCPConnection &) = delete;
    
    ~TCPConnection();
};