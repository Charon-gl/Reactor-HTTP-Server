#include "TCPConnection.hpp"

TCPConnection::TCPConnection(int fd, EventLoop *_eventloop) : eventloop(_eventloop), channel(std::make_shared<Channel>(fd, eventloop))
{
    // 绑定Channel的事件回调, 默认监听读事件
    channel->set_read_callback(std::bind(&TCPConnection::handle_reading, this));
    channel->set_write_callback(std::bind(&TCPConnection::handle_writing, this));
    channel->set_error_callback(std::bind(&TCPConnection::disconnect, this));
    channel->enable_reading();
}

void TCPConnection::handle_reading()
{
    while(1)
    {
        recv_buf.clear();
        int len = recv(channel->get_fd(), recv_buf.data(), recv_buf.size() - 1, 0);
        if(len <= 0)
        {
            if(len <= -1 )
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                { 
                    std::cout << std::endl;
                    break;
                }
                else
                {

                }
            }
            else    // = 0
            {
                std::cout << "客户端已断开连接..." << std::endl;
                disconnect();
            }
            }
        else
        {
            recv_buf += '\0';
            std::cout << recv_buf;
        }
    }
}

void TCPConnection::handle_writing()
{
    int len = send(channel->get_fd(), write_buf.data(), write_buf.size() - 1, 0);
    if(len == -1)
    {

    }
    channel->disbale_writing();     //发送完数据就关闭写监听
}

void TCPConnection::disconnect()
{
    int ret = epoll_ctl(eventloop->get_epfd(), EPOLL_CTL_DEL, channel->get_fd(), NULL);
    if(ret == -1)
    {

    }
    close(channel->get_fd());
    channel = nullptr;
}

void TCPConnection::pre_send(const std::string& data)
{
    snprintf(write_buf.data(), data.size() - 1, data.data());
    if (!channel->is_writing_enabled())
        channel->enable_writing();
}

std::shared_ptr<Channel> TCPConnection::get_channel() const
{
    return channel;
}