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
#include "tcp_accessor.h"
#include "connector_mgr.h"

TcpListener::TcpListener() : m_sockfd(0), m_epfd(0), m_conn_mgr(NULL) {}

TcpListener::~TcpListener() {}

int TcpListener::Init(int epfd, ConnectorMgr* conn_mgr)
{
    if (!conn_mgr)
    {
        LogError("conn_mgr can not be null");
        return -1;
    }
    m_epfd = epfd;
    m_conn_mgr = conn_mgr;
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd <= 0)
    {
        LogError("socket failed. m_sockfd:%d, errno:%d", m_sockfd, errno);
        return -1;
    }
    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_len = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(6008);
    if (bind(m_sockfd, (struct sockaddr*)&server_addr, sock_len) < 0)
    {
        LogError("bind error");
        return -1;
    }
    if (listen(m_sockfd, 5) < 0)
    {
        LogError("listen error");
        return -1;
    }
    // set nonblocking
    SetNonBlocking(m_sockfd);
    // add to epoll
    EpollEvent ev = {0};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = this;
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_sockfd, &ev) < 0)
    {
        LogError("add m_sockfd %d to epoll failed", m_sockfd);
        return -1;
    }
    return 0;
}

int TcpListener::Update()
{
    if (m_events & EPOLLIN)
    {
        if (Accept() < 0)
        {
            LogError("recv new connection error");
        }
        m_events &= (~EPOLLIN);
    }
    return 0;
}

int TcpListener::Accept()
{
    struct sockaddr_in client_addr;
    socklen_t sock_len = sizeof(client_addr);
    int fd = accept(m_sockfd, (struct sockaddr*)(&client_addr), 
                    &sock_len);
    std::cout<<"new connection come from:"<<inet_ntoa(client_addr.sin_addr)<<",fd:"<<fd<<std::endl;
    LogDebug("new connection come from:%s, fd:%d", inet_ntoa(client_addr.sin_addr), fd);
    if (fd < 0)
    {
        LogError("accept error, m_sockfd:%d, retfd:%d", m_sockfd, fd);
        return -1;
    }
    m_conn_mgr->OnNewTcpConnect(fd);
    return 0;
}

int TcpListener::Fini()
{
    EpollEvent ev = {0};
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_sockfd, &ev) < 0)
    {
        LogError("delete TcpListener from epoll failed");
    }
    close(m_sockfd);
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


