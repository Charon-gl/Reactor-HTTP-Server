#include "Acceptor.hpp"

Acceptor::Acceptor(uint16_t port, EventLoop* el) : lfd(-1) 
{
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int ret = bind(lfd, (sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {

    }
    ret = listen(lfd, 1024);
    if (ret == -1)
    {

    }
}

Acceptor& Acceptor::instance(uint16_t port, EventLoop* el)
{
    static Acceptor acceptor(port, el);
    return acceptor;
}

int Acceptor::get_lfd() const { return lfd; }

void Acceptor::accept_fd()
{
    sockaddr_in caddr;
    memset(&caddr, 0, sizeof(caddr));
    socklen_t len = sizeof(caddr);
    int cfd = accept(lfd, (sockaddr *)&caddr, &len);
    if(cfd == -1)
    {
        
    }
    add_client_callback;
}

void Acceptor::set_add_client(std::function<void(int)> _cb)
{
    add_client_callback = std::move(_cb);
}