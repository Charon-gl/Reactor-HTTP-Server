#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <errno.h>
#include <functional>
#include "ThreadPool.hpp"
#include "Channel.hpp"
#include "HTTP_Analysis.hpp"
#include "Err_Manager.hpp"

#define MAX_BUF_SIZE 1024
class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
private:
    std::shared_ptr<Channel> channel;
    ThreadPool *threadpool;
    sockaddr_in addr;
    socklen_t *len;
    std::string recv_buf;
    std::string write_buf;
    size_t write_buf_len;
    size_t pre_pos;
    bool set_write_listen;
    bool write_shutdown;
    int _errno;

    std::function<void(std::function<void()>)> add_task_and_call_main_thread;
    std::function<void(int, int)> disconnect_callback; // 绑定的是Server的del_client()

public:
    TCPConnection(int, ThreadPool*, std::function<void(std::shared_ptr<Channel>&)>);

    int handle_reading();
    int handle_writing();

    void set_add_task_and_call_main_thread(std::function<void(std::function<void()>)>);
    void set_disconnect(std::function<void(int, int)>);

    void pre_send(const std::string&);     //将数据写入缓冲区，并注册写事件

    std::shared_ptr<Channel> get_channel() const;
    TCPConnection(TCPConnection &&) = delete;
    TCPConnection(const TCPConnection &) = delete;
    TCPConnection &operator=(TCPConnection &&) = delete;
    TCPConnection &operator=(const TCPConnection &) = delete;
    
    ~TCPConnection();
};