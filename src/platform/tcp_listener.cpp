#include "tcp_listener.h"
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
#include "tcp_connector.h"

TcpListener::TcpListener() : _sockfd(0), _epfd(0), _processor(NULL) {}

TcpListener::~TcpListener() {}

int TcpListener::Init(int epfd, App* processor)
{
    LogDebug("TcpListener Init");
    if (!processor)
    {
        LogError("processor can not be null");
        return -1;
    }
    _epfd = epfd;
    _processor = processor;
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sockfd <= 0)
    {
        LogError("socket failed. _sockfd:%d, errno:%d", _sockfd, errno);
        return -1;
    }
    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_len = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(6008);
    if (bind(_sockfd, (struct sockaddr*)&server_addr, sock_len) < 0)
    {
        LogError("bind error");
        return -1;
    }
    if (listen(_sockfd, 5) < 0)
    {
        LogError("listen error");
        return -1;
    }
    // set nonblocking
    int opts = fcntl(_sockfd, F_GETFL);
    if (opts < 0)
    {
        LogError("get fcntl failed");
        return -1;
    }
    opts |= O_NONBLOCK;
    if (fcntl(_sockfd, F_SETFL, opts) < 0)
    {
        LogError("set fcntl error");
        return -1;
    }
    // add to epoll
    EpollEvent ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = this;
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockfd, &ev) < 0)
    {
        LogError("add _sockfd %d to epoll failed", _sockfd);
        return -1;
    }
    return 0;
}

int TcpListener::Update()
{
    if (_events & EPOLLIN)
    {
        if (Recv() < 0)
        {
            LogError("recv new connection error");
        }
    }
    std::list<Connector*>::iterator itor = _connector_list.begin();
    for (; itor != _connector_list.end(); ++itor)
    {
        if (*itor)
        {
            (*itor)->Update();
            // remove disconnected connector
            if ((*itor)->GetStatus() == CONNECTOR_STATUS_DISCONNECTED)
            {
                LogDebug("client disconnected");
                itor = _connector_list.erase(itor);
            }
        }
    }
    return 0;
}

int TcpListener::Send(void* buf, int buf_len)
{
    // do nothing
    return 0;
}

int TcpListener::Recv()
{
    struct sockaddr_in client_addr;
    socklen_t sock_len = sizeof(client_addr);
    int fd = accept(_sockfd, (struct sockaddr*)(&client_addr), 
                    &sock_len);
    LogDebug("new connection come from:%s, fd:%d", inet_ntoa(client_addr.sin_addr), fd);
    _events &= (~EPOLLIN);
    if (fd < 0)
    {
        LogError("accept error, _sockfd:%d, retfd:%d", _sockfd, fd);
        return -1;
    }
    TcpConnector* conn = new TcpConnector;
    if (!conn)
    {
        LogError("new access come, but failed to new TcpConnector");
        return -1;
    }
    conn->Init(fd, _epfd, _processor);
    _connector_list.push_back(conn);
    return 0;
}

int TcpListener::Fini()
{
    EpollEvent ev = {0};
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, _sockfd, &ev) < 0)
    {
        LogError("delete TcpListener from epoll failed");
    }
    close(_sockfd);
    std::list<Connector*>::iterator itor = _connector_list.begin();
    for (; itor != _connector_list.end(); ++itor)
    {
        if (*itor)
        {
            (*itor)->Fini();
            delete (*itor);
            (*itor) = NULL;
        }
    }
    _connector_list.clear();
    return 0;
}


