#include "Server.hpp"

Server::Server()
    : port(0), eventloop(&EventLoop::instance()), acceptor(nullptr) {}

void Server::add_client(int fd)
{
    clients.emplace(std::pair<int, std::unique_ptr<TCPConnection>>(fd, std::make_unique<TCPConnection>(fd, [this](std::shared_ptr<Channel> new_channel){ 
        eventloop->add_new_channel(new_channel); 
    })));
    clients[fd]->set_disconnect([this](int _fd, int _errno) {
        del_client(_fd, _errno); 
    });
}

void Server::del_client(int fd, int _errno)
{
    //Logger::add_log(err_to_string(_errno), _errno, fd);
    eventloop->del_channel(fd);
    clients.erase(fd);
}

void Server::del_all(int _errno)
{
    //Logger::add_log(err_to_string(_errno), _errno, -1, -1);     //最后一个-1表示全局错误
    while(!clients.empty())     //逐个删除，沿用已有的释放链
    {
        auto it = clients.begin();
        int _fd = it->second->get_channel()->get_fd();
        del_client(_fd, _errno);
    }
}

Server &Server::instance()
{
    static Server server;
    return server;
}

void Server::set_port(uint16_t _port) { port = _port; }

bool Server::create()
{
    acceptor = &Acceptor::instance(eventloop);
    int ret = acceptor->init_listen_fd(port);
    if(ret == -1)
    {
        //Logger::add_log("服务器初始化： 监听套接字初始化失败", -1);
        return false;
    }
    eventloop->set_acceptor(acceptor);  //先将acceptor给到eventloop再创建epoll实例
    bool res = eventloop->init();
    if(!res)
    {
        //Logger::add_log("服务器初始化： epoll实例创建失败", -1);
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