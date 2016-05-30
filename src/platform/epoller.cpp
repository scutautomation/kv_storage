#include "epoller.h"
#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "log.h"

Acceptor::Acceptor(int fd, EventBuffer* read_buf, EventBuffer* write_buf) : _fd(fd), 
         _read_buf(read_buf), _write_buf(write_buf), _epoller(NULL)
{

}

Acceptor::~Acceptor()
{

}

int Acceptor::OnCanRead(EpollEvent ev)
{
    LogInfo("sockfd:%d, new fd:%d", _fd, ev.data.fd);
    if (ev.data.fd == _fd)
    {
        LogInfo("recv new connection");
        return Accept();
    }
    LogInfo("can read from %d", ev.data.fd);
    if (!_read_buf)
    {
        LogError("read buffer is null");
        return -1;
    }
    //todo:读取固定大小数据放入bufer，没有数据时读取失败需要关闭fd
    return _read_buf->Push();
}

int Acceptor::OnCanWrite(EpollEvent ev)
{
    if (!_write_buf)
    {
        LogError("write buffer is null");
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
                    &sock_len);
    EpollEvent ev;
    ev.data.fd = fd;
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
    if (!_acceptor)
    {
        return -1;
    }
    _acceptor->SetEpoller(this);
    _events = new EpollEvent[kMaxEpollEvent];
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
        _events = NULL;
    }
    close(_epfd);
}

int Epoller::Add(int fd, EpollEvent ev)
{
    LogInfo("add fd:%d, ev fd:%d", fd, ev.data.fd);
    return epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
}

int Epoller::Del(int fd, EpollEvent ev)
{
    return epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Mod(int fd, EpollEvent ev)
{
    return epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

int Epoller::Dispatch()
{
    int evnum = epoll_wait(_epfd, _events, kMaxEpollEvent, -1);
    for (int i = 0; i < evnum; ++i)
    {
        LogInfo("fd is %d", _events[i].data.fd);
        if (_events[i].events & EPOLLIN)
        {
            _acceptor->OnCanRead(_events[i]);
        }
        else if (_events[i].events & EPOLLOUT)
        {
            _acceptor->OnCanWrite(_events[i]);
        }
        else
        {
            // std::cout<<"can not route events"
        }
    }
}
