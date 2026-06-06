#include "EventLoop.hpp"
#include "Acceptor.hpp"

EventLoop EventLoop::eventloop;

bool EventLoop::init()
{
    int _epfd = epoll_create(1);
    if(_epfd == -1) 
    { 
        //std::cerr << "Epoll_create failed" << std::endl;
        return false;
    }
    epfd = std::make_unique<Channel>(_epfd);

    int lfd = acceptor->get_lfd()->get_fd();
    epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN | EPOLLET;
    
    int flag = fcntl(lfd, F_GETFL);
    fcntl(lfd, F_SETFL, flag | O_NONBLOCK);
    
    while(1)
    {
        int ret = epoll_ctl(epfd->get_fd(), EPOLL_CTL_ADD, lfd, &ev);
        if(ret == -1)
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_WAIT, _errno, lfd);
            if (res == Err_Manager::Action_Callback::IGNORE)
                return true;
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
            {
                channels[lfd]->disconnect_callback(_errno);
                return false;
            }
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                call_close_all(_errno);
                return false;
            }
        }
        break;
    }
    channels.emplace(std::pair<int, std::shared_ptr<Channel>>(lfd, acceptor->get_lfd()));
    return true;
}

EventLoop& EventLoop::instance() { return eventloop; }

EventLoop::EventLoop() : acceptor(nullptr), evs(std::vector<epoll_event>(1024)), is_stop(false), _errno(0) {}

void EventLoop::set_acceptor(Acceptor *&_acceptor) { acceptor = _acceptor; }

void EventLoop::loop()
{
    while(!is_stop)
    {
        int nums_fd = epoll_wait(epfd->get_fd(), evs.data(), sizeof(evs) / sizeof(evs[0]), -1);
        if(nums_fd < 0)
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_WAIT, _errno);
            if(res == Err_Manager::Action_Callback::RETRY)
                continue;
            if(res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                call_close_all(_errno);
                return;
            }
        }
        for (int i = 0; i < nums_fd; ++i)
        {
            /*if(evs[i].data.fd == acceptor->get_lfd()->get_fd())     //将acceptor的连接任务放到读事件里完成，只需顺序分配即可
                acceptor->accept_fd();
            else
            {*/
                int res = channels[evs[i].data.fd]->event_handle(evs[i].events);
                if(res == -1)   //表示发生了严重错误，需要关闭全站
                    is_stop = true;
        }
    }
    call_close_all(_errno);
    return;
}

int EventLoop::get_epfd() const { return epfd->get_fd(); }

void EventLoop::update_Channel(Channel*& ch)
{
    int fd = ch->get_fd();
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = ch->get_events();
    while(1)
    {
        int ret = epoll_ctl(epfd->get_fd(), EPOLL_CTL_MOD, fd, &ev);
        if(ret == -1)
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_CTL, _errno, fd);
            if(res == Err_Manager::Action_Callback::IGNORE)
                return;
            if(res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
            {
                ch->disconnect_callback(_errno);
                return;
            }
            if(res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                is_stop = true;
                return;
            }
        }
        break;
    }
    if(ev.events & EPOLLOUT)
        ch->set_writing_enabled(true);
    else if(ev.events & ~EPOLLOUT)
        ch->set_writing_enabled(false);
}

void EventLoop::add_new_channel(const std::shared_ptr<Channel>& _ptr)
{
    int fd = _ptr->get_fd();
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = _ptr->get_events();
    while(1)
    {
        int ret = epoll_ctl(epfd->get_fd(), EPOLL_CTL_ADD, fd, &ev);
        if (ret == -1)
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_CTL, _errno, fd);
            if (res == Err_Manager::Action_Callback::IGNORE)
                return;
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
            {
                channels[fd]->disconnect_callback(_errno);
                return;
            }
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                is_stop = true;
                return;
            }
        }
        break;
    }
    auto it = channels.emplace(std::pair<int, std::shared_ptr<Channel>>(fd, _ptr));
    it.first->second->set_update_events([this](Channel *ch){ 
        this->update_Channel(ch); 
    });
}

void EventLoop::del_channel(int fd)
{
    while(1)
    {
        int ret = epoll_ctl(epfd->get_fd(), EPOLL_CTL_DEL, fd, NULL);
        if (ret == -1)
        {
            _errno = errno;
            auto res = Err_Manager::err_judge(Err_Manager::Action_Type::EPOLL_CTL, _errno, fd);
            if (res == Err_Manager::Action_Callback::IGNORE)
                //主要是ENOENT,表示待删除fd不在epoll实例中，直接返回即可
                return;
            if (res == Err_Manager::Action_Callback::RETRY)
                continue;
            if (res == Err_Manager::Action_Callback::CLOSE_FD)
            {//该语句针对EINVAL错误，因为del不会触发ECONNRESET
                epoll_event tmp_ev;
                int test = epoll_wait(epfd->get_fd(), &tmp_ev, 1, 0);
                if(test >= 0)   //epfd正常上班，待删除fd触发EINVAL
                    break;
                else 
                    is_stop = true;
                return;
            }
            if (res == Err_Manager::Action_Callback::CLOSE_ALL)
            {
                is_stop = true;
                return;
            }
        }
        break;
    }
    channels.erase(fd);
}

void EventLoop::set_call_close_all(std::function<void(int)> _cb) { call_close_all = std::move(_cb); }