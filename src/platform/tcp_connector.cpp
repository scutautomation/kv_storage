#include "tcp_connector.h"
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

TcpConnector::TcpConnector() : _sockfd(0), _epfd(0), _processor(NULL) {}
TcpConnector::~TcpConnector() {}

int TcpConnector::Init(int sockfd, App* processor)
{
    // do nothing
    return 0;
}

int TcpConnector::Init(int sockfd, int epfd, App* processor)
{
    _sockfd = sockfd;
    _processor = processor;
    _epfd = epfd;
    EpollEvent ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.ptr = this;
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockfd, &ev) < 0)
    {
        LogError("epoll_ctl failed");
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
    memset(&_recv_buffer, 0, sizeof(_recv_buffer));
    _recv_buffer.buf_len = kMaxEventBufferLen;
    _recv_buffer.buf = (char*)malloc(kMaxEventBufferLen);
    if (!_recv_buffer.buf)
    {
        LogError("create event buffer failed");
        return -1;
    }

    _send_buffer.buf_len = kMaxEventBufferLen;
    _send_buffer.buf = (char*)malloc(kMaxEventBufferLen);
    if (!_send_buffer.buf)
    {
        LogError("create event buffer failed");
        return -1;
    }

    _conn_head.ptr = this;
    _status = CONNECTOR_STATUS_CONNECTED;
    return 0;
}

int TcpConnector::Update()
{
    int ret = 0;
    if (_events & EPOLLIN)
    {
        ret =  Recv();
        if (ret != 0)
        {
            LogError("recv data failed");
            return -1;
        }
        ret = _processor->OnRecvClient(_conn_head, _recv_buffer.buf, _recv_buffer.data_len);
    }
    return ret;
}

int TcpConnector::Recv()
{
    int length = 0;
    // todo: bugs here, should read by pos
    while (1)
    {
        int bytes = recv(_sockfd, _recv_buffer.buf, _recv_buffer.buf_len - 1, 0);
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
    _events &= (~EPOLLIN);
    if (length < _recv_buffer.buf_len)
    {
        _recv_buffer.buf[length] = '\0';
        _recv_buffer.data_len = length;
    }
    else
    {
        _recv_buffer.buf[0] = '\0';
        _recv_buffer.data_len = 0;
        _recv_buffer.start_pos = 0;
        _recv_buffer.end_pos = 0;
        LogError("buffer data too long, size:%d, max_allow_size:%d", length, _recv_buffer.buf_len);
        return -1;
    }
    return 0;
}

int TcpConnector::Send(void* buf, int buf_len)
{
    if (_events & EPOLLOUT)
    {
        while(1)
        {
            size_t len = send(_sockfd, buf, buf_len, 0);
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
        _events &= (~EPOLLOUT);
    }
    return 0;
}

void TcpConnector::FiniSock()
{
    EpollEvent ev = {0};
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, _sockfd, &ev) < 0)
    {
        LogError("epoll_ctl del fd failed, fd:%d", _sockfd);
    }
    close(_sockfd);
    _status = CONNECTOR_STATUS_DISCONNECTED;
}

int TcpConnector::Fini()
{
    FiniSock();
    return 0;
}