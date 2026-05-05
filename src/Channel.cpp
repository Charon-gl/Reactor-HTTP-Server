#include "Channel.hpp"

Channel::Channel(int _fd, EventLoop* _eventLoop) 
    : fd(_fd), events(EPOLLOUT | EPOLLET), eventloop(_eventLoop), writing_enabled(false) 
{
    int flag = fcntl(fd, F_GETFL);
    fcntl(fd, flag | O_NONBLOCK);
    eventloop->add_new_channel(this);
}

void Channel::set_read_callback(std::function<void()> _cb)
{
    read_callback = std::move(_cb);
}

void Channel::set_write_callback(std::function<void()> _cb)
{
    write_callback = std::move(_cb);
}

void Channel::set_error_callback(std::function<void()> _cb)
{
    error_callback = std::move(_cb);
}

void Channel::event_handle(uint32_t revents)
{
    if(events == 0)
        return;
    if(revents | EPOLLIN)
        read_callback;
    if(revents | EPOLLOUT)
        write_callback;
    if(revents | EPOLLERR)
        error_callback;
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
}