#include "connector_mgr.h"
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

#include "tcp_listener.h"
#include "log.h"

ConnectorMgr::ConnectorMgr() : _epfd(0), _events(0)
{

}

ConnectorMgr::~ConnectorMgr() {}

int ConnectorMgr::Init(App* processor)
{
    //if (!processor)
    //{
    //    LogError("processor can not be null");
    //    return -1;
    //}
    _epfd = epoll_create(1024);
    if (_epfd <= 0)
    {
        LogError("epoll_create failed");
        return -1;
    }
    int ret = 0;
    TcpListener* tcp_listener = NULL;
    do
    {
        _events = new EpollEvent[kMaxEpollEvent];
        if (!_events)
        {
            LogError("failed to new EpollEvent");
            ret = -1;
            break;
        }
        tcp_listener = new TcpListener;
        if (!tcp_listener)
        {
            LogError("failed to new TcpListener");
            ret = -1;
            break;
        }
        LogDebug("tcp listener start to init");
        int ret = tcp_listener->Init(_epfd, processor);
        if (ret != 0)
        {
            LogError("tcp_listener init failed");
            ret = -1;
            break;
        }
    } while(0);
    if (ret != 0)
    {
        if (_events)
        {
            delete _events;
            _events = NULL;
        }
        if (tcp_listener)
        {
            delete tcp_listener;
        }
        return -1;
    }
    _access_list.push_back(tcp_listener);
    return 0;
}

int ConnectorMgr::Update()
{
    int num = epoll_wait(_epfd, _events, kMaxEpollEvent, -1);
    if (num < 0)
    {
        //if (errno != EINTR)
        //{
        //    LogError("wait epoll failed");
        //    return -1;
        //}
        LogError("epoll_wait failed, ret:%d", num);
        return -1;
    }
    LogDebug("active num:%d", num);
    // update all access event
    for (int i = 0; i < num; ++i)
    {
        Connector* conn = (Connector*)_events[i].data.ptr;
        if (conn)
        {
            conn->SetEvents(_events[i].events);
        }
    }
    // update connectors of different type(tcp, udp)
    std::list<Connector*>::iterator itor = _access_list.begin();
    for (; itor != _access_list.end(); ++itor)
    {
        if (*itor)
        {
            (*itor)->Update();
        }
    }
    return 0;
}

void ConnectorMgr::CloseConnectors()
{
    std::list<Connector*>::iterator itor = _access_list.begin();
    for (; itor != _access_list.end(); ++itor)
    {
        if (*itor)
        {
            (*itor)->Fini();
        }
        delete (*itor);
        (*itor) = NULL;
    }
    _access_list.clear();
}

int ConnectorMgr::Fini()
{
    close(_epfd);
    if (_events)
    {
        delete[] _events;
        _events = NULL;
    }
}

