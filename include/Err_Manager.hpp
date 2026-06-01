#pragma once

#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <functional>

class Err_Manager
{
public:
    enum class Action_Type
    {
        EPOLL_WAIT,
        EPOLL_CTL,
        ACCEPT,
        READ_OR_WRITE,
    };

    enum class Action_Callback
    {
        IGNORE,    // 忽略
        RETRY,     // 重试
        CLOSE_FD,  // 关闭客户fd
        CLOSE_ALL, // 释放全部连接
    };

    Err_Manager(const Err_Manager &) = delete;
    Err_Manager(Err_Manager &&) = delete;

    static Action_Callback err_judge(Action_Type, int _errno, int _fd = -1);

private:
    Err_Manager() = default;

    static Action_Callback epoll_wait_err(int);
    static Action_Callback epoll_ctl_err(int, int _fd = -1);
    static Action_Callback accept_err(int);
    static Action_Callback read_or_write_err(int);
};