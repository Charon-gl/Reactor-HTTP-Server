#include "Server.hpp"

Server::Server(const std::string& _log_file_)
    : port(0), eventloop(&EventLoop::instance()), acceptor(nullptr), logger(&Logger::instance(_log_file_)) , threadpool(std::make_unique<ThreadPool>(NUM_THREADS)) {}

void Server::add_client(int fd)
{
    clients.emplace(std::pair<int, std::shared_ptr<TCPConnection>>(fd, std::make_shared<TCPConnection>(fd, threadpool.get())));
    clients[fd]->init();
    eventloop->add_new_channel(clients[fd]->get_channel());
    clients[fd]->get_channel()->enable_reading();

    clients[fd]->set_add_task_and_call_main_thread([this](std::function<void()> _cb) {
        eventloop->enqueue(std::move(_cb));
        eventfd_write(eventloop->get_eventfd(), static_cast<eventfd_t>(1)); // 这是为结束主线程在epoll_wait的阻塞用的
    });

    clients[fd]->set_disconnect([this](int _fd, int _errno) { 
        eventloop->enqueue([this, _fd, _errno] {    //把断开连接的操作也丢到任务队列，避免工作线程执行时因为资源释放导致错误
            del_client(_fd, _errno);
        });
    });
}

void Server::del_client(int fd, int _errno)
{
    const char *log_str = err_to_string(_errno);
    if (_errno == 0)
        logger->add_log(Logger::LOG_RANK::INFO, log_str, _errno, fd);
    else
        logger->add_log(Logger::LOG_RANK::ERROR, log_str, _errno, fd);
    eventloop->del_channel(fd);
    clients.erase(fd);
}

void Server::del_all(int _errno)
{
    logger->add_log(Logger::LOG_RANK::FATAL, err_to_string(_errno), _errno);
    while (!clients.empty()) // 逐个删除，沿用已有的释放链
    {
        auto it = clients.begin();
        int _fd = it->second->get_channel()->get_fd();
        del_client(_fd, _errno);
    }
}

Server &Server::instance(const std::string &_log_file_)
{
    static Server server(_log_file_);
    return server;
}

void Server::set_port(uint16_t _port) { port = _port; }

bool Server::create()
{
    acceptor = &Acceptor::instance(eventloop);
    int ret = acceptor->init_listen_fd(port);
    if(ret == -1)
    {
        logger->add_log(Logger::LOG_RANK::FATAL, "服务器初始化： 监听套接字初始化失败", -1);
        return false;
    }
    eventloop->set_acceptor(acceptor);  //先将acceptor给到eventloop再创建epoll实例
    bool res = eventloop->init();
    if(!res)
    {
        logger->add_log(Logger::LOG_RANK::FATAL, "服务器初始化： EventLoop创建失败", -1);
        return false;
    }

    //绑定回调
    acceptor->set_add_client([this](int cfd) { 
        this->add_client(cfd); 
    });
    eventloop->set_call_close_all([this](int _errno) {
        this->del_all(_errno);
    });
    return true;
}

bool Server::run(uint16_t _port) 
{
    set_port(_port);
    bool ret = create();
    if(!ret)
        return false;
    eventloop->loop();
    return true;
}