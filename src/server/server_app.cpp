#include "server_app.h"
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "platform/log.h"
#include <stdio.h>
#include <string.h>

ServerApp::ServerApp(Envs envs) : App(envs), _conn_mgr(NULL)
{

}

ServerApp::~ServerApp()
{

}

int ServerApp::Init(const Envs& envs)
{
    _conn_mgr = new ConnectorMgr;
    if (!_conn_mgr)
    {
        LogError("failed to new ConnectorMgr");
        return -1;
    }
    int ret = _conn_mgr->Init(this);
    if (ret < 0)
    {
        LogError("failed to init _conn_mgr");
        return -1;
    }
    return 0;
}

int ServerApp::Proc(const Envs& envs)
{
    if (!_conn_mgr)
    {
        return -1;
    }
    std::cout<<"proc"<<std::endl;
    return _conn_mgr->Update();
}

int ServerApp::OnRecvClient(ConnHead conn_head, void* buf, int32_t buf_len)
{
    LogInfo("______recv from client:%s", (char*)buf);
    char sbuf[1024];
    snprintf(sbuf, sizeof(sbuf), "_____from server");
    // todo: pack and unpack
    SendToClient(conn_head, sbuf, (int32_t)strlen(sbuf));
    return 0;
}

int ServerApp::Tick(const Envs& envs)
{
    LogInfo("server %d tick", envs.app_id);
    return 0;
}

int ServerApp::Fini(const Envs& envs)
{
    if (_conn_mgr)
    {
        _conn_mgr->CloseConnectors();
        delete _conn_mgr;
        _conn_mgr = NULL;
    }
    return 0;
}
