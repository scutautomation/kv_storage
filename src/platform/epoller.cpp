#include "epoller.h"

Acceptor::Acceptor(int fd, EventBuffer* read_buf, EventBuffer* write_buf) : _fd(fd), 
         _read_buf(read_buf), _write_buf(write_buf), _epoller(NULL)
{

}

Acceptor::~Acceptor()
{

}

int Acceptor::OnCanRead(EpollEvent ev)
{
    if (ev.fd == _fd)
    {
        return Accept();
    }
    if (!_read_buf)
    {
        return -1;
    }
    return _read_buf->Push();
}

int Acceptor::OnCanWrite(EpollEvent ev)
{
    if (!_write_buf)
    {
        return -1;
    }
    // pop data, and write
    return _write_buf->Pop();
}

int Acceptor::Accept()
{
    if (!_epoller)
    {
        return -1;
    }
    struct sockaddr_in client_addr;
    socklen_t sock_len = sizeof(client_addr);
    int fd = accept(_fd, reinterpret_cast<struct sockaddr*>(&client_addr), 
                    &sock_len)
    EpollEvent ev;
    ev.fd = fd;
    ev.events = EPOLLIN;
    _epoller->Add(fd, ev);
    return 0;
}

Epoller::Epoller(Acceptor* acceptor) : _epfd(0), _events(NULL), _acceptor(acceptor)
{

}

Epoller::~Epoller()
{

}

int Epoller::Create()
{
    typedef struct epoll_event EEVENT;
    _events = new EEVENT[kMaxEpollEvent];
    if (!_events)
    {
        return -1;
    }
    _epfd = epoll_create(1024);
    if (_epfd <= 0)
    {
        return -1;
    }
    return 0;
}

void Epoller::Destroy()
{
    if (_events)
    {
        delete[] _events;
    }
    close(_epfd);
}

struct epoll_event Epoller::EventTranslate(EpollEvent ev)
{
    struct epoll_event ep_ev;
    ep_ev.fd = ev.fd;
    ep_ev.data.ptr = ev.callback;
    ep_ev.events = ev.events;
    return ep_ev;
}

int Epoller::Add(int fd, EpollEvent ev)
{
    struct epoll_event ep_ev = EventTranslate(ev);
    return epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ep_ev);
}

int Epoller::Del(int fd, EpollEvent ev)
{
    struct epoll_event ep_ev = EventTranslate(ev);
    return epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ep_ev);
}

int Epoller::Mod(int fd, EpollEvent ev)
{
    struct epoll_event ep_ev = EventTranslate(ev);
    return epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ep_ev);
}

int Epoller::Dispatch()
{
    int evnum = epoll_wait(_epfd, _events, kMaxEpollEvent, -1);
    for (int i = 0; i < evnum; ++i)
    {
        EpollEvent ev;
        ev.fd = _events[i].fd;
        ev.callback = _events[i].data.ptr;
        ev.events = _events[i].events;
        if (_events[i].events & EPOLLIN)
        {
            if (_acceptor)
            {
                _acceptor.OnCanRead(ev);
            }
        }
        else if (_events[i].events & EPOLLOUT)
        {
            if (_acceptor)
            {
                _acceptor.OnCanWrite(ev);
            }
        }
        else
        {
            // std::cout<<"can not route events"
        }
    }
}
