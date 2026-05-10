#include "TCPConnection.hpp"

TCPConnection::TCPConnection(int fd, EventLoop *_eventloop) : eventloop(_eventloop), channel(std::make_shared<Channel>(fd, eventloop)), write_buf_len(0), pre_pos(0)
{
    // 绑定Channel的事件回调, 默认监听读事件
    eventloop->add_new_channel(channel);//建议包装成function
    channel->set_read_callback(std::bind(&TCPConnection::handle_reading, this));
    channel->set_write_callback(std::bind(&TCPConnection::handle_writing, this));
    channel->set_disconnect_callback([this](int _errno) { 
        this->disconnect_callback(channel->get_fd(), _errno); 
    });
    channel->enable_reading();
}

int TCPConnection::handle_reading()
{
    while (errno != EAGAIN || errno != EWOULDBLOCK)//优化死循环
    {
        recv_buf.clear();
        int len = recv(channel->get_fd(), recv_buf.data(), recv_buf.size() - 1, 0);
        if(len <= 0)
        {
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::READ_OR_WRITE, errno);
            if (res == Err_Manager::Action_Callback::IGNORE)
                return;
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
            {
                disconnect_callback(channel->get_fd(), errno);
                return;
            }
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                eventloop->call_close_all(errno);
                return;
            }
            return 0;
        }
        recv_buf += '\0';
        std::cout << recv_buf;
    }
    std::cout << std::endl;
    return -1;
}

int TCPConnection::handle_writing()
{//需要考虑缓冲区大小是否足以将全部数据一次发送出去
    while(write_buf_len > 0)
    {
        int len = send(channel->get_fd(), write_buf.data() + pre_pos, write_buf_len, 0);
        if(len > 0)
        {
            write_buf_len -= len;
            pre_pos += len;
        }
        else
        {
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::READ_OR_WRITE, errno);
            if (res == Err_Manager::Action_Callback::IGNORE)
            {
                pre_send(write_buf);
                return -1;
            }
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
                return 0;
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                eventloop->call_close_all(errno);
                return -1;
            }
            return 0;
        }
    }
    channel->disbale_writing();     //发送完数据就关闭写监听
    return -1;
}

void TCPConnection::set_disconnect(std::function<void(int, int)> _cb) { disconnect_callback = std::move(_cb); }

void TCPConnection::pre_send(const std::string& data)
{
    write_buf.clear();
    pre_pos = 0;
    snprintf(write_buf.data(), data.size() - 1, data.data());
    write_buf_len = write_buf.size();
    if (!channel->is_writing_enabled())
        channel->enable_writing();
}

std::shared_ptr<Channel> TCPConnection::get_channel() const { return channel; }

TCPConnection::~TCPConnection() { eventloop->del_channel(channel->get_fd()); }// 建议包装成function 