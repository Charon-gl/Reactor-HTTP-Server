#include "Server.hpp"

void Server::add_client(int fd)
{
    clients.emplace(std::pair<int, std::unique_ptr<TCPConnection>>(fd, std::make_unique<TCPConnection>(fd, eventloop)));
}

void Server::del_client(int fd)
{
    clients.erase(fd);
}

Server &Server::instance()
{
    return server;
}

void Server::create()
{
    eventloop->instance();
    acceptor->instance(port, eventloop);
    acceptor->set_add_client([this](int cfd){ 
        this->add_client(cfd); 
    });
}

void Server::run()
{
    eventloop->loop();
}

Server::Server(uint16_t _port) 
    : port(_port), eventloop(nullptr), acceptor(nullptr) {}

Server::~Server()
{

}
