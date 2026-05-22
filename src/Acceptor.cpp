#include "Acceptor.hpp"

Acceptor::Acceptor(EventLoop* _eventloop) 
    : eventloop(_eventloop) {}

Acceptor &Acceptor::instance(EventLoop *_eventloop)
{
    static Acceptor acceptor(_eventloop);
    return acceptor;
}

int Acceptor::init_listen_fd(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        std::cerr << "Socket failed" << std::endl;
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int ret = bind(fd, (sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        std::cerr << "Bind failed" << std::endl;
        close(fd);
        return -1;
    }
    ret = listen(fd, 1024);
    if (ret == -1)
    {
        std::cerr << "Listen failed" << std::endl;
        close(fd);
        return -1;
    }
    lfd = std::make_unique<Channel>(fd, eventloop);

    return fd;
}

int Acceptor::get_lfd() const { return lfd->get_fd(); }

void Acceptor::accept_fd()
{
    sockaddr_in caddr;
    memset(&caddr, 0, sizeof(caddr));
    socklen_t len = sizeof(caddr);
    int cfd = accept(lfd->get_fd(), (sockaddr *)&caddr, &len);
    if(cfd == -1) 
    {
        auto res = Err_Manager::err_judge(Err_Manager::Action_Type::READ_OR_WRITE, errno);
        if (res == Err_Manager::Action_Callback::IGNORE)
            return;
        if (res == Err_Manager::Action_Callback::RETRY)
        {   
            cfd = accept(lfd->get_fd(), (sockaddr *)&caddr, &len);
            if (cfd == -1)
                auto res = Err_Manager::err_judge(Err_Manager::Action_Type::READ_OR_WRITE, errno);
        }
        if (res == Err_Manager::Action_Callback::CLOSE_ALL)
        {
            eventloop->call_close_all(errno);
            return;
        }
    }
    add_client_callback(cfd);
}

void Acceptor::set_add_client(std::function<void(int)> _cb) { add_client_callback = std::move(_cb); }