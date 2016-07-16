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
#include "tcp_accessor.h"
#include "tcp_listener.h"
#include "log.h"

ConnectorMgr::ConnectorMgr() : m_epfd(0), m_events(0), m_processor(NULL)
{

}

ConnectorMgr::~ConnectorMgr() {}

int ConnectorMgr::Init(App* processor)
{
    if (!processor)
    {
        LogError("processor can no be null");
        return -1;
    }
    m_processor = processor;
    m_epfd = epoll_create(1024);
    if (m_epfd <= 0)
    {
        LogError("epoll_create failed");
        return -1;
    }
    m_events = new EpollEvent[kMaxEpollEvent];
    if (!m_events)
    {
        LogError("failed to new EpollEvent");
        return -1;
    }    
    return 0;
}

TcpListener* ConnectorMgr::CreateTcpListener()
{
    TcpListener* tcp_listener = NULL;
    tcp_listener = new TcpListener;
    if (!tcp_listener)
    {
        LogError("failed to new TcpListener");
        return NULL;
    }
    int ret = tcp_listener->Init(m_epfd, this);
    if (ret != 0)
    {
        LogError("tcp_listener init failed");
        delete tcp_listener;
        return NULL;
    }
    m_listener_list.push_back(tcp_listener);
    return tcp_listener;
}

TcpAccessor* ConnectorMgr::CreateTcpAccessor(int fd)
{
    TcpAccessor* conn = new TcpAccessor;
    if (!conn)
    {
        LogError("new access come, but failed to new TcpAccessor");
        return NULL;
    }
    int ret = conn->Init(fd, m_epfd, m_processor);
    if (ret != 0)
    {
        LogError("new connection init failed");
        delete conn;
        return NULL;
    }
    m_access_list.push_back(conn);
    return conn;
}

void ConnectorMgr::OnNewTcpConnect(int fd)
{
    (void)CreateTcpAccessor(fd);
}

int ConnectorMgr::Update()
{
    if (m_access_list.size() == 0 && m_listener_list.size() == 0)
    {
        LogDebug("no network active, pass");
        return -1;
    }
    int num = epoll_wait(m_epfd, m_events, kMaxEpollEvent, -1);
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
        Connector* conn = (Connector*)m_events[i].data.ptr;
        if (conn)
        {
            conn->SetEvents(m_events[i].events);
        }
    }
    // update listeners
    std::list<NetListener*>::iterator listener_itor = m_listener_list.begin();
    for (; listener_itor != m_listener_list.end(); ++listener_itor)
    {
        if (*listener_itor)
        {
            (*listener_itor)->Update();
        }
    }
    // update connectors
    std::list<NetAccessor*>::iterator acc_itor = m_access_list.begin();
    for (; acc_itor != m_access_list.end(); ++acc_itor)
    {
        if (*acc_itor)
        {
            (*acc_itor)->Update();
            // remove disconnected connector
            if ((*acc_itor)->GetStatus() == CONNECTOR_STATUS_DISCONNECTED)
            {
                std::cout<<"client disconnected"<<std::endl;
                LogDebug("client disconnected");
                (*acc_itor)->Fini();
                delete (*acc_itor);
                (*acc_itor) = NULL;
                acc_itor = m_access_list.erase(acc_itor);
            }
        }
    }
    return 0;
}

int ConnectorMgr::Fini()
{
    close(m_epfd);
    if (m_events)
    {
        delete[] m_events;
        m_events = NULL;
    }

    std::list<NetAccessor*>::iterator acc_itor = m_access_list.begin();
    for (; acc_itor != m_access_list.end(); ++acc_itor)
    {
        if (*acc_itor)
        {
            (*acc_itor)->Fini();
            delete (*acc_itor);
            (*acc_itor) = NULL;
        }        
    }
    m_access_list.clear();

    std::list<NetListener*>::iterator listener_itor = m_listener_list.begin();
    for (; listener_itor != m_listener_list.end(); ++listener_itor)
    {
        if (*listener_itor)
        {
            (*listener_itor)->Fini();
            delete (*listener_itor);
            (*listener_itor) = NULL;
        }        
    }
    m_listener_list.clear();
    return 0;
}

