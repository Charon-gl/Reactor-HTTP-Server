#include "err_to_string.hpp"

char* err_to_string(int _errno)
{
    if(_errno == EINTR)
        return "系统中断";
    else if(_errno == EAGAIN || _errno == EWOULDBLOCK)
        return "内核缓冲区为空或已满";
    else if(_errno == EEXIST)
        return "fd已存在于epoll";
    else if (_errno == ECONNABORTED) 
        return "客户端在未建立连接前主动断开";
    else if(_errno == EOF)
        return "客户端正常断开连接";
    else if (_errno == ENOENT)
        return "文件或目录不存在";
    else if(_errno == ECONNRESET)
        return "客户端强行断开连接";
    else if(_errno == ETIMEDOUT)
        return "连接超时";
    else if(_errno == ENETUNREACH)
        return "网络不可达";
    else if(_errno == EHOSTUNREACH)
        return "主机不可达";
    else if (_errno == EPIPE)
        return "往已断开的连接/管道写数据";
    else if(_errno == ENOSPC)
        return "epoll最大监听数已满";
    else if(_errno == EBADF)
        return "无效文件描述符";
    else if (_errno == EMFILE)
        return "进程打开fd达到上限";
    else if (_errno == ENFILE)
        return "系统全局已无fd可用";
    else if (_errno == ENOBUFS)
        return "内核缓冲区不足";
    else if (_errno == ENOMEM)
        return "内存不足";
    else if (_errno == EINVAL)
        return "参数无效";
    return "未知错误";
}