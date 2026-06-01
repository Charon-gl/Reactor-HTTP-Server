#include "Channel.hpp"

Channel::Channel(int _fd) 
    : fd(_fd), events(EPOLLIN | EPOLLET), writing_enabled(false) 
{
    int flag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void Channel::set_read_callback(std::function<int()> _cb) { read_callback = std::move(_cb); }

void Channel::set_write_callback(std::function<int()> _cb) { write_callback = std::move(_cb); }

void Channel::set_disconnect_callback(std::function<void(int)> _cb) { disconnect_callback = std::move(_cb); }

void Channel::set_update_events(std::function<void(Channel*)> _cb) { update_events = std::move(_cb); }

int Channel::event_handle(uint32_t revents)
{
    if(revents == 0)
        return 1;
    
    if(revents & EPOLLIN)
    {
        int ret = read_callback();
        if(ret >= 0)
        {
            disconnect_callback(ret);
            return 0;
        }
        else if(ret == -1)
            return -1;
    }
    
    if (revents & EPOLLOUT)
    {
        if(write_callback)
        {
            int ret = write_callback();
            //如果返回等于0，表示一次发不完，因此不能先关闭写端，要等待下一次写事件
            if(ret == -100)
                shutdown(fd, SHUT_WR);  //关闭写端(主动给客户端发送FIN，进入半关闭状态，但仍可以接收数据)
            else if (ret > 0)
            {
                disconnect_callback(ret); //fd异常，需要断开连接
                return 0;
            }
            else if(ret == -1)
                return -1;
        }
    }

    if (revents & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        if(disconnect_callback)
        {
            int err = 0;
            socklen_t len = sizeof(err);
            getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
            if (err == 0)
                err = -1; // 表示未知错误
            disconnect_callback(err);
            return 0;
        }
    }
    return 1;
}

bool Channel::enable_reading()
{
    events |= EPOLLIN;
    update_events(this);
    return true;
}

bool Channel::enable_writing()
{
    events |= EPOLLOUT;
    update_events(this);
    return true;
}

bool Channel::disbale_writing()
{
    events &= ~EPOLLOUT;
    update_events(this);
    return true;
}

bool Channel::is_writing_enabled() const { return writing_enabled; }

void Channel::set_writing_enabled(bool choice) { writing_enabled = choice; }

/*bool Channel::clear_events()
{
    events = 0;
    update_events(this);
    return true;
}*/

int Channel::get_fd() const { return fd; }
uint32_t Channel::get_events() const { return events; }

Channel::~Channel()
{
    if(fd != -1)
        close(fd);
    fd = -1;
}