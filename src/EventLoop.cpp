#include "EventLoop.hpp"

void EventLoop::init()
{
    epfd = epoll_create(1);
    if(epfd == -1)
    {

    }
    int lfd = acceptor->get_lfd();
    epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN | EPOLLET;
    int flag = fcntl(lfd, F_GETFL);
    fcntl(lfd, F_SETFL, flag | O_NONBLOCK);
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {

    }
}

EventLoop& EventLoop::instance()
{
    return eventloop;
}

EventLoop::EventLoop() : epfd(-1), acceptor(nullptr)
{
    init();
}

void EventLoop::set_acceptor(Acceptor *_acceptor) { acceptor = _acceptor; }

void EventLoop::loop()
{
    while(1)
    {
        int nums_fd = epoll_wait(epfd, evs.data(), sizeof(evs) / sizeof(evs[0]), -1);
        for (int i = 0; i < nums_fd; ++i)
        {
            if(evs[i].data.fd = acceptor->get_lfd())
                acceptor->accept_fd();
            else
                channels[evs[i].data.fd]->event_handle(evs[i].events);
        }
    }
}

int EventLoop::get_epfd() const { return epfd; }

void EventLoop::update_Channel(Channel* ch)
{
    int fd = ch->get_fd();
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = ch->get_events();
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    if(ret == -1)
    {

    }
    if(ev.events | EPOLLOUT)
        ch->set_writing_enabled(true);
    else if(ev.events | ~EPOLLOUT)
        ch->set_writing_enabled(false);
}

void EventLoop::add_new_channel(Channel* channel)
{
    int fd = channel->get_fd();
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = channel->get_events();
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    channels.emplace(std::pair<int, Channel *>(fd, channel));
}

void EventLoop::del_channel(int fd)
{
    channels.erase(fd);
}