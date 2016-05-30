#include "server_app.h"
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "platform/log.h"
#include "platform/epoller.h"

ServerApp::ServerApp(Envs envs) : App(envs)
{

}

ServerApp::~ServerApp()
{

}

int ServerApp::Init(const Envs& envs)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_len = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(6008);
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&server_addr), sock_len) < 0)
    {
        LogError("bind error");
        return -1;
    }
    if (listen(sockfd, 5) < 0)
    {
        LogError("listen error");
        return -1;
    }
    _acceptor = new Acceptor(sockfd, NULL, NULL);
    if (!_acceptor)
    {
        LogError("new Acceptor failed");
        return -1;
    }
    _epoller = new Epoller(_acceptor);
    if (!_epoller)
    {
        LogError("new Epoller failed");
        return -1;
    }
    int ret = _epoller->Create();
    if (ret != 0)
    {
        LogError("epoller Create failed");
        return -1;
    }
    EpollEvent ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    return _epoller->Add(sockfd, ev);
}

int ServerApp::Proc(const Envs& envs)
{
    return _epoller->Dispatch();
}

int ServerApp::Tick(const Envs& envs)
{
    LogInfo("server %d tick", envs.app_id);
    return 0;
}

int ServerApp::Fini(const Envs& envs)
{
    LogInfo("server %d fini", envs.app_id);
    if (_epoller)
    {
        _epoller->Destroy();
        delete _epoller;
        delete _acceptor;
        _acceptor = NULL;
        _epoller = NULL;
    }
    return 0;
}
