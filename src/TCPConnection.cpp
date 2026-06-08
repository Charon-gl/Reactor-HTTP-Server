#include "TCPConnection.hpp"

TCPConnection::TCPConnection(int fd, ThreadPool* _threadpool) 
    : channel(std::make_shared<Channel>(fd)), threadpool(_threadpool)
{
    recv_buf.reserve(MAX_BUF_SIZE);
    recv_buf_len = MAX_BUF_SIZE;
    recv_pre_pos = 0;
    write_buf.reserve(MAX_BUF_SIZE);
    write_buf_len = MAX_BUF_SIZE;
    write_pre_pos = 0;
    set_write_listen = false;
    write_shutdown = false;
    _errno = 0;
}

void TCPConnection::init()
{
    std::weak_ptr<TCPConnection> weak_connection = shared_from_this();
    // 绑定Channel的事件回调, 默认监听读事件
    channel->set_read_callback([weak_connection] { 
        auto shared_connection = weak_connection.lock();
        if(!shared_connection)
            return 0;
        return shared_connection->handle_reading(); 
    });

    auto shared_channel = channel;
    channel->set_write_callback([weak_connection, shared_channel] { 
        auto shared_connection = weak_connection.lock();
        if(!shared_connection || !shared_channel || shared_channel->get_fd()< 0)
            return 0;
        return shared_connection->handle_writing(); 
    });

    channel->set_disconnect_callback([weak_connection](int _errno) {
        auto shared_connection = weak_connection.lock();
        if(shared_connection) 
        shared_connection->disconnect_callback(shared_connection->get_channel()->get_fd(), _errno); 
    });
}

int TCPConnection::handle_reading()
{
    if(recv_buf.size() < MAX_BUF_SIZE)
        recv_buf.resize(MAX_BUF_SIZE);
    recv_buf_len = recv_buf.size();
    recv_pre_pos = 0;

    while (1)
    {
        int len = recv(channel->get_fd(), recv_buf.data() + recv_pre_pos, recv_buf_len, 0);
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
            return 0;

        recv_buf_len -= len;
        recv_pre_pos += len;
    }
    recv_buf += '\0';

    if (!write_shutdown) // 写端关闭后，不再受理任何请求
    {//HTTP解析与生成响应报文的任务交给线程池
        const std::weak_ptr<TCPConnection> weak_self = shared_from_this();
        auto task = [weak_self]
        {
            auto shared_self = weak_self.lock(); // 尝试升级成shared_ptr
            if (!shared_self)                    // 表示连接已被释放
                return;

            std::string response_data = HTTP_Analysis::package(shared_self->recv_buf);

            auto channel_ = shared_self->channel;
            //加入主线程的任务队列，并通知主线程
            auto task_to_main_thread = [weak_self, channel_, _response_data = std::move(response_data)] // 传右值接管内存，避免拷贝开销
            {
                auto shared_self = weak_self.lock();    //尝试升级成shared_ptr
                if (!shared_self || !channel_)
                    return;
                shared_self->pre_send(channel_, _response_data);
            };
            shared_self->add_task_and_call_main_thread(std::move(task_to_main_thread));
            return;
        };

        threadpool->add_task(std::move(task));
    }
    return -100; // 无特殊意义，仅表示读回调正常完成
}

int TCPConnection::handle_writing()
{
    // 需要考虑缓冲区大小是否足以将全部数据一次发送出去
    while(write_buf_len > 0)
    {
        int len = send(channel->get_fd(), write_buf.data() + write_pre_pos, write_buf_len, 0);
        if(len > 0)
        {
            write_buf_len -= len;
            write_pre_pos += len;
        }
        if(len == 0)
            return 0;
        else
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::READ_OR_WRITE, _errno);
            if (res == Err_Manager::Action_Callback::IGNORE)
                return -10; //表示写缓冲区的数据还没发送完，不能关闭写端
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
                return _errno;
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
                return -1;
        }
    }
    write_shutdown = true;
    return -100; //表示写缓冲区的内容已全部发送
}

void TCPConnection::set_add_task_and_call_main_thread(std::function<void(std::function<void()>)> _cb) { add_task_and_call_main_thread = std::move(_cb); }
void TCPConnection::set_disconnect(std::function<void(int, int)> _cb) { disconnect_callback = std::move(_cb); }

void TCPConnection::pre_send(std::shared_ptr<Channel> channel_, const std::string& data)
{
    if(!channel_ || channel_->get_fd() < 0)
        return;
    write_pre_pos = 0;
    write_buf = data;
    write_buf_len = write_buf.size();
    int res = handle_writing();     //尝试发送
    if(res == -10)
    {
        if (!channel_->is_writing_enabled())
            channel_->enable_writing();
    }
    else if (res == -100)
        shutdown(channel_->get_fd(), SHUT_WR); // 关闭写端(主动给客户端发送FIN，进入半关闭状态，但仍可以接收数据)
    else if (res >= 0)
        disconnect_callback(channel_->get_fd(), res); // 断开连接
}

std::shared_ptr<Channel> TCPConnection::get_channel() const { return channel; }

TCPConnection::~TCPConnection() {}