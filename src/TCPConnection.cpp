#include "TCPConnection.hpp"

TCPConnection::TCPConnection(int fd, std::function<void(std::shared_ptr<Channel>&)> add_client_callback) 
    : channel(std::make_shared<Channel>(fd)), write_buf_len(0), pre_pos(0), set_write_listen(false), write_shutdown(false), _errno(errno)
{
    //添加channel到epoll实例
    add_client_callback(channel);

    // 绑定Channel的事件回调, 默认监听读事件
    channel->set_read_callback(std::bind(&TCPConnection::handle_reading, this));
    channel->set_write_callback(std::bind(&TCPConnection::handle_writing, this));
    channel->set_disconnect_callback([this](int _errno) { 
        disconnect_callback(channel->get_fd(), _errno); 
    });
    recv_buf.resize(MAX_BUF_SIZE);
    write_buf.resize(MAX_BUF_SIZE);

    //开启监听
    channel->enable_reading();
}

int TCPConnection::handle_reading()
{
    while (1)
    {
        if(recv_buf.size() < MAX_BUF_SIZE)
            recv_buf.resize(MAX_BUF_SIZE);
        int len = recv(channel->get_fd(), recv_buf.data(), recv_buf.size(), 0);
        if (len < 0)
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::READ_OR_WRITE, _errno);
            if (res == Err_Manager::Action_Callback::IGNORE)
                break;
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
                return _errno;  //fd错误，非正常断开连接
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
                return -1;
        }
        else if(len == 0)
            return 0;   //收到FIN报文，正常关闭
        recv_buf += '\0';
    }
    if (!write_shutdown) // 写端关闭后，不再受理任何请求
        pre_send(HTTP_Analysis::package(recv_buf));
    return -100;    //无特殊意义，仅表示读回调正常完成
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
                return 0; //表示写缓冲区的数据还没发送完，不能关闭写端
            }
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
                return _errno;
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
                return -1;
        }
    }
    channel->disbale_writing(); // 发送完数据就关闭写监听
    write_shutdown = true;
    return -100; //表示写缓冲区的内容已全部发送
}

void TCPConnection::set_disconnect(std::function<void(int, int)> _cb) { disconnect_callback = std::move(_cb); }

void TCPConnection::pre_send(const std::string& data)
{
    pre_pos = 0;
    write_buf = data;
    write_buf_len = write_buf.size();
    if (!channel->is_writing_enabled())
        channel->enable_writing();
}

std::shared_ptr<Channel> TCPConnection::get_channel() const { return channel; }

TCPConnection::~TCPConnection()
{
    // 直接清空该连接的事件回调，避免段错误
    channel->set_read_callback(nullptr);
    channel->set_write_callback(nullptr);
    channel->set_disconnect_callback(nullptr);
}