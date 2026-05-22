#include "Server.hpp"

void Server::add_client(int fd)
{
    clients.emplace(std::pair<int, std::unique_ptr<TCPConnection>>(fd, std::make_unique<TCPConnection>(fd, eventloop)));
    clients[fd]->set_disconnect([this](int _fd, int _errno) {
        this->del_client(_fd, _errno); 
    });
}

void Server::del_client(int fd, int _errno)
{
    Logger::add_log(err_to_string(_errno), fd);
    clients.erase(fd);
    eventloop->del_channel(fd);
}

void Server::del_all(int _errno)
{
    Logger::add_log(err_to_string(_errno), -1, -1);     //最后一个-1表示全局错误
    for (auto &i : clients)
        clients.erase(i.first);
}

Server &Server::instance()
{
    static Server server;
    return server;
}

void Server::set_port(uint16_t _port) { port = _port; }

void Server::create()
{
    acceptor = &Acceptor::instance(eventloop);
    eventloop->set_acceptor(acceptor);
    eventloop->set_call_close_all([this](int _errno) {
        this->del_all(_errno);
    });
    int ret = acceptor->init_listen_fd(port);
    if(ret == -1) { exit(1); }  //要释放所有资源再结束
    acceptor->set_add_client([this](int cfd) { 
        this->add_client(cfd); 
    });
}

void Server::run(uint16_t _port) 
{
    set_port(port);
    create();
    eventloop->loop(); 
}

Server::Server() 
    : port(0), eventloop(&EventLoop::instance()), acceptor(nullptr) {}
