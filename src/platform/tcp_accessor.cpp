#include "tcp_accessor.h"
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
#include "app.h"

TcpAccessor::TcpAccessor() : m_sockfd(0), m_epfd(0), m_processor(NULL) {}
TcpAccessor::~TcpAccessor() {}

int TcpAccessor::Init(int sockfd, int epfd, App* processor)
{
    if (processor == NULL)
    {
        LogError("processor can not be null");
        return -1;
    }
    m_sockfd = sockfd;
    m_processor = processor;
    m_epfd = epfd;
    EpollEvent ev = {0};
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.ptr = this;
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_sockfd, &ev) < 0)
    {
        LogError("epoll_ctl failed");
        return -1;
    }
    // set nonblocking
    SetNonBlocking(m_sockfd);
    memset(&m_recv_buffer, 0, sizeof(m_recv_buffer));
    m_recv_buffer.buf_len = kMaxEventBufferLen;
    m_recv_buffer.buf = new char[kMaxEventBufferLen];
    if (!m_recv_buffer.buf)
    {
        LogError("create event buffer failed");
        return -1;
    }

    m_send_buffer.buf_len = kMaxEventBufferLen;
    m_send_buffer.buf = new char[kMaxEventBufferLen];
    if (!m_send_buffer.buf)
    {
        LogError("create event buffer failed");
        return -1;
    }

    m_conn_head.ptr = this;
    m_status = CONNECTOR_STATUS_CONNECTED;
    return 0;
}

int TcpAccessor::Update()
{
    int ret = 0;
    if (m_events & EPOLLIN)
    {
        ret =  Recv();
        if (ret != 0)
        {
            LogError("recv data failed");
            return -1;
        }
        ret = m_processor->OnRecvClient(m_conn_head, m_recv_buffer.buf, m_recv_buffer.data_len);
    }
    return ret;
}

int TcpAccessor::Recv()
{
    int length = 0;
    // todo: bugs here, should read by pos
    while (1)
    {
        int bytes = recv(m_sockfd, m_recv_buffer.buf, m_recv_buffer.buf_len - 1, 0);
        if (bytes < 0)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            bytes = 0;
        }
        // client closed
        else if (bytes == 0)
        {
            FiniSock();
            return 0;
        }
        length += bytes;
    }
    m_events &= (~EPOLLIN);
    if (length < m_recv_buffer.buf_len)
    {
        m_recv_buffer.buf[length] = '\0';
        m_recv_buffer.data_len = length;
    }
    else
    {
        m_recv_buffer.buf[0] = '\0';
        m_recv_buffer.data_len = 0;
        m_recv_buffer.start_pos = 0;
        m_recv_buffer.end_pos = 0;
        LogError("buffer data too long, size:%d, max_allow_size:%d", length, m_recv_buffer.buf_len);
        return -1;
    }
    return 0;
}

int TcpAccessor::Send(void* buf, int buf_len)
{
    if (m_events & EPOLLOUT)
    {
        while(1)
        {
            size_t len = send(m_sockfd, buf, buf_len, 0);
            if (len == 0)
            {
                FiniSock();
                return 0;
            }
            else if (len < 0)
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                else
                {
                    LogError("send data failed");
                    return -1;
                }
            }
            break;
        }
        m_events &= (~EPOLLOUT);
    }
    return 0;
}

void TcpAccessor::FiniSock()
{
    EpollEvent ev = {0};
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_sockfd, &ev) < 0)
    {
        LogError("epoll_ctl del fd failed, fd:%d", m_sockfd);
    }
    close(m_sockfd);
    m_status = CONNECTOR_STATUS_DISCONNECTED;
}

int TcpAccessor::Fini()
{
    FiniSock();
    if (m_recv_buffer.buf)
    {
        delete[] m_recv_buffer.buf;
        memset(&m_recv_buffer, 0, sizeof(m_recv_buffer));
    }
    if (m_send_buffer.buf)
    {
        delete[] m_send_buffer.buf;
        memset(&m_send_buffer, 0, sizeof(m_send_buffer));
    }
    return 0;
}