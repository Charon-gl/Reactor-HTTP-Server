#include "EventLoop.hpp"

bool EventLoop::init()
{
    epfd = epoll_create(1);
    if(epfd == -1)  //是否要交给errmanager
    { 
        std::cerr << "Epoll_create failed" << std::endl;
        return false;
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
        auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_WAIT, errno);
        if (res == Err_Manager::Action_Callback::IGNORE)
            return;
        if (res == Err_Manager::Action_Callback::RETRY)
            epoll_ctl(epfd, EPOLL_CTL_MOD, lfd, &ev);
        if (res == Err_Manager::Action_Callback::CLOSE_FD)
        {
            channels[lfd]->do_close(errno);
            return;
        }
        if (res == Err_Manager::Action_Callback::CLOSE_ALL)
        {
            for (auto &i : channels)
                i.second->do_close(errno);
            return;
        }
    }
    return true;
}

EventLoop& EventLoop::instance() { return eventloop; }

EventLoop::EventLoop() : epfd(-1), acceptor(nullptr) { init(); }

void EventLoop::set_acceptor(Acceptor *_acceptor) { acceptor = _acceptor; }

void EventLoop::loop()
{
    while(1)//考虑是否可以让上层主动结束事件循环
    {
        int nums_fd = epoll_wait(epfd, evs.data(), sizeof(evs) / sizeof(evs[0]), -1);
        if(nums_fd < 0)
        {
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_WAIT, errno);
            if(res == Err_Manager::Action_Callback::RETRY)
                continue;
            if(res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                for (auto &i : channels)
                    i.second->do_close(errno);
                return;
            }
        }
        for (int i = 0; i < nums_fd; ++i)
        {
            if(evs[i].data.fd == acceptor->get_lfd())
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
        auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_CTL, errno, fd);
        if(res == Err_Manager::Action_Callback::IGNORE)
            return;
        if(res == Err_Manager::Action_Callback::RETRY)
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
        if(res == Err_Manager::Action_Callback::CLOSE_FD)
        {
            ch->do_close(errno);
            return;
        }
        if(res == Err_Manager::Action_Callback::CLOSE_ALL)
        {
            call_close_all(errno);
            return;
        }
    }
    if(ev.events | EPOLLOUT)
        ch->set_writing_enabled(true);
    else if(ev.events | ~EPOLLOUT)
        ch->set_writing_enabled(false);
}

void EventLoop::add_new_channel(std::shared_ptr<Channel> _ptr)
{
    int fd = _ptr->get_fd();
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = _ptr->get_events();
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1)
    {
        auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_CTL, errno, fd);
        if (res == Err_Manager::Action_Callback::IGNORE)
            return;
        if (res == Err_Manager::Action_Callback::RETRY)
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
        if (res == Err_Manager::Action_Callback::CLOSE_FD)
        {
            channels[fd]->do_close(errno);
            return;
        }
        if (res == Err_Manager::Action_Callback::CLOSE_ALL)
        {
            for (auto &i : channels)
                i.second->do_close(errno);
            return;
        }
    }
    channels.emplace(std::pair<int, std::shared_ptr<Channel>>(fd, _ptr));
}

void EventLoop::del_channel(int fd)
{
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    if (ret == -1)
    {
        auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_CTL, errno, fd);
        if (res == Err_Manager::Action_Callback::IGNORE)
            return;
        if (res == Err_Manager::Action_Callback::RETRY)
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd, NULL);
        if (res == Err_Manager::Action_Callback::CLOSE_FD)
        {
            channels[fd]->do_close(errno);
            return;
        }
        if (res == Err_Manager::Action_Callback::CLOSE_ALL)
        {
            call_close_all(errno);
            return;
        }
    }
    channels.erase(fd);
}

void EventLoop::set_call_close_all(std::function<void(int)> _cb) { call_close_all = std::move(_cb); }