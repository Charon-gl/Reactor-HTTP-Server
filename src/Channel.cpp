#include "Channel.hpp"

Channel::Channel(int _fd, EventLoop* _eventLoop) 
    : fd(_fd), events(EPOLLOUT | EPOLLET), eventloop(_eventLoop), writing_enabled(false) 
{
    int flag = fcntl(fd, F_GETFL);
    fcntl(fd, flag | O_NONBLOCK);
}

void Channel::set_read_callback(std::function<int()> _cb) { read_callback = std::move(_cb); }

void Channel::set_write_callback(std::function<int()> _cb) { write_callback = std::move(_cb); }

void Channel::set_disconnect_callback(std::function<void(int)> _cb) { disconnect_callback = std::move(_cb); }

void Channel::do_close(int _errno) { disconnect_callback(_errno); }

void Channel::event_handle(uint32_t revents)
{
    if(revents == 0)
        return;
    if(revents | EPOLLIN)
    {
        int ret = read_callback();
        if(ret >= 0)
        {
            do_close(errno);
            return;
        }
    }
    if(revents & (EPOLLERR || revents | EPOLLHUP || revents | EPOLLRDHUP))
    {
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
        if(err == 0)
            err = -1;   //表示未知错误
        do_close(err);
        return;
    }
    if (revents | EPOLLOUT)
    {
        int ret = write_callback();
        if (ret >= 0)
        {
            do_close(errno);
            return;
        }
    }
}

bool Channel::enable_reading()
{
    events |= EPOLLIN;
    update();
    return true;
}

bool Channel::enable_writing()
{
    events |= EPOLLOUT;
    update();
    return true;
}

bool Channel::disbale_writing()
{
    events |= ~EPOLLOUT;
    update();
    return true;
}

bool Channel::is_writing_enabled() const { return writing_enabled; }

void Channel::set_writing_enabled(bool choice) { writing_enabled = choice; }

bool Channel::enable_error()
{
    events |= EPOLLERR;
    update();
    return true;
}

bool Channel::clear_events()
{
    events = 0;
    update();
    return true;
}

int Channel::get_fd() const { return fd; }
uint32_t Channel::get_events() const { return events; }

void Channel::update()
{
    eventloop->update_Channel(this);
}

Channel::~Channel()
{
    if(fd != -1)
        close(fd);
    fd = -1;
}