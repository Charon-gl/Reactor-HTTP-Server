#include "Err_Manager.hpp"

Err_Manager::Action_Callback Err_Manager::epoll_wait_err(int _errno)
{
    if (_errno == EINTR)
        return Action_Callback::RETRY;

    return Action_Callback::CLOSE_ALL;
}

Err_Manager::Action_Callback Err_Manager::epoll_ctl_err(int _errno, int _fd)
{
    if (_errno == EEXIST || _errno == ENOENT || _errno == EOF)
        return Action_Callback::IGNORE;
    if (_errno == EINTR)
        return Action_Callback::RETRY;
    if (_errno == ECONNRESET || _errno == EINVAL)
        return Action_Callback::CLOSE_FD;
    if (_errno == EBADF)    //单EBADF无法判断是epfd的问题还是fd的问题
        return _fd == -1 ? Action_Callback::CLOSE_FD : Action_Callback::CLOSE_ALL;

    return Action_Callback::CLOSE_ALL;
}

Err_Manager::Action_Callback Err_Manager::accept_err(int _errno)
{
    if (_errno == ECONNABORTED || _errno == EMFILE || _errno == ENOSPC || _errno == ECONNRESET)
        return Action_Callback::IGNORE;
    if (_errno == EINTR)
        return Action_Callback::RETRY;
    if (_errno = EAGAIN || _errno == EWOULDBLOCK)
        return Action_Callback::CLOSE_FD;
        
    return Action_Callback::CLOSE_ALL;
}

Err_Manager::Action_Callback Err_Manager::read_or_write_err(int _errno)
{
    if (_errno == EAGAIN || _errno == EWOULDBLOCK)
        return Action_Callback::IGNORE;
    if (_errno == EINTR)
        return Action_Callback::RETRY;
    if (_errno == EOF || _errno == ENOENT || _errno == ECONNRESET || _errno == EPIPE || _errno == EINVAL || _errno == EBADF)
        return Action_Callback::CLOSE_FD;

    return Action_Callback::CLOSE_ALL;
}

Err_Manager::Action_Callback Err_Manager::err_judge(Err_Manager::Action_Type type, int _errno, int _fd)
{
    if(type == Action_Type::EPOLL_WAIT)
        return epoll_wait_err(_errno);

    if(type == Action_Type::EPOLL_CTL)
        return epoll_ctl_err(_errno, _fd);

    if (type == Action_Type::ACCEPT)
        return accept_err(_errno);

    if (type == Action_Type::READ_OR_WRITE)
        return read_or_write_err(_errno);
        
    return Action_Callback::CLOSE_ALL;
}