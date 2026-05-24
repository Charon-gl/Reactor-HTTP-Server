#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <errno.h>
#include <functional>
#include "Channel.hpp"
#include "HTTP_Analysis.hpp"
#include "Err_Manager.hpp"

class TCPConnection
{
private:
    std::shared_ptr<Channel> channel;
    sockaddr_in addr;
    socklen_t *len;
    std::string recv_buf;
    std::string write_buf;
    size_t write_buf_len;
    size_t pre_pos;
    bool set_write_listen;
    bool write_shutdown;

    std::function<void(int, int)> disconnect_callback; // 绑定的是Server的del_client()

public:
    TCPConnection(int, std::function<void(std::shared_ptr<Channel>&)>);

    int handle_reading();
    int handle_writing();

    void set_disconnect(std::function<void(int, int)>);

    void pre_send(const std::string&);     //将数据写入缓冲区，并注册写事件

    std::shared_ptr<Channel> get_channel() const;
    TCPConnection(TCPConnection &&) = delete;
    TCPConnection(const TCPConnection &) = delete;
    TCPConnection &operator=(TCPConnection &&) = delete;
    TCPConnection &operator=(const TCPConnection &) = delete;
    
    ~TCPConnection();
};