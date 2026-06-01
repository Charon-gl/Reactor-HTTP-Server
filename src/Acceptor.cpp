#include "Acceptor.hpp"
#include "EventLoop.hpp"

Acceptor::Acceptor(EventLoop*& _eventloop) 
    : eventloop(_eventloop) {}

Acceptor &Acceptor::instance(EventLoop*& _eventloop)
{
    static Acceptor acceptor(_eventloop);
    return acceptor;
}

int Acceptor::init_listen_fd(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        //std::cerr << "Socket failed" << std::endl;
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int ret = bind(fd, (sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        //std::cerr << "Bind failed" << std::endl;
        close(fd);
        return -1;
    }
    ret = listen(fd, 1024);
    if (ret == -1)
    {
        //std::cerr << "Listen failed" << std::endl;
        close(fd);
        return -1;
    }
    lfd = std::make_shared<Channel>(fd);

    lfd->set_read_callback(std::bind(&Acceptor::accept_fd, this));

    return fd;
}

std::shared_ptr<Channel> Acceptor::get_lfd() const { return lfd; }

int Acceptor::accept_fd()
{
    sockaddr_in caddr;
    memset(&caddr, 0, sizeof(caddr));
    socklen_t len = sizeof(caddr);
    int cfd = -1;
    while (1)
    {
        cfd = accept(lfd->get_fd(), (sockaddr *)&caddr, &len);
        if(cfd == -1) 
        {
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::ACCEPT, errno);
            if (res == Err_Manager::Action_Callback::IGNORE)
                return 0;
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
                return -1;
        }
        break;
    }
    if(cfd != -1)
        add_client_callback(cfd);
    return 1;
}

void Acceptor::set_add_client(std::function<void(int)> _cb) { add_client_callback = std::move(_cb); }