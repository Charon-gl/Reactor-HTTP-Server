#include "Server.hpp"

void Server::init()
{
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        throw std::runtime_error("Socket failed");
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8888);
    int ret = bind(lfd, (sockaddr *)&addr, sizeof(addr));
    if(ret == -1)
    {
        throw std::runtime_error("Bind failed");
    }
    ret = listen(lfd, 1024);
    if(ret == -1)
    {
        throw std::runtime_error("Listen error");
    }
}

void Server::run()
{
    epfd = epoll_create(1024);
    //添加lfd到epfd
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = lfd;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        throw std::runtime_error("Add lfd into epoll failed");
    }

    epoll_event evs[1024];
    while (1)
    {
        int nums_fd = epoll_wait(epfd, evs, sizeof(evs) / sizeof(evs[0]), -1);
        for (int i = 0; i < nums_fd; ++i)
        {
            int fd = evs[i].data.fd;
            if(fd == lfd)
            {
                while(1)
                {
                    if(errno == EAGAIN)
                        break;
                    int cfd = accept(lfd, NULL, NULL);
                    if(cfd == -1)
                    {
                        std::cerr << "Accept new client failed" << std::endl;
                        continue;
                    }
                    ev.events = EPOLLIN;
                    ev.data.fd = cfd;
                    int flag = fcntl(cfd, F_GETFD);
                    fcntl(cfd, F_SETFD, flag | EPOLLET);
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                    if(ret == -1)
                    {
                        std::cerr << "Add cfd into epoll failed" << std::endl;
                        close(cfd);
                    }
                    cfds.insert(cfd);
                }
            }
            else
            {
                while(1)
                {
                    if(errno = EAGAIN)
                        break;
                    char buf[1024];
                    int len = recv(fd, buf, sizeof(buf) - 1, 0);
                    if(len == -1)
                    {
                        std::cerr << "Recv from " << fd << " failed" << std::endl;
                        break;
                    }
                    else if(len == 0)
                    {
                        int del = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        if(del == -1)
                        {
                            std::cerr << "Delete client from epoll failed" << std::endl;
                            //还需其他处理
                        }
                        std::cout << "Client disconnected..." << std::endl;
                        close(fd);
                    }
                    else 
                    {
                        buf[len] = '\0';
                        std::cout << buf << std::endl;  //多线程要注意
                        int sd = send(fd, buf, len, 0);
                        if(sd == -1)
                        {
                            std::cerr << "Send failed" << std::endl;
                            int del = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            if (del == -1)
                            {
                                std::cerr << "Delete client from epoll failed" << std::endl;
                                // 还需其他处理
                            }
                            std::cout << "Client disconnected..." << std::endl;
                            cfds.erase(fd);
                            close(fd);
                        }
                    }
                }
            }
        }
    }
}

Server::Server() : lfd(-1), epfd(-1)
{
    init();
    run();
}

Server::~Server()
{
    if(epfd != -1)
        close(epfd);
    if(lfd != -1)
        close(lfd);
}
