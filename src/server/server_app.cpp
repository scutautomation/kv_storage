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

ServerApp::ServerApp(Envs envs) : App(envs), m_conn_mgr(NULL)
{

}

ServerApp::~ServerApp()
{

}

int ServerApp::Init(const Envs& envs)
{
    m_conn_mgr = new ConnectorMgr;
    if (!m_conn_mgr)
    {
        LogError("failed to new ConnectorMgr");
        return -1;
    }
    int ret = m_conn_mgr->Init(this);
    if (ret < 0)
    {
        LogError("failed to init m_conn_mgr");
        return -1;
    }
    if (m_conn_mgr->CreateTcpListener() ==  NULL)
    {
        LogError("failed to create tcp listener");
        return -1;
    }
    return 0;
}

int ServerApp::Proc(const Envs& envs)
{
    if (!m_conn_mgr)
    {
        return -1;
    }
    return m_conn_mgr->Update();
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
    if (m_conn_mgr)
    {
        m_conn_mgr->Fini();
        delete m_conn_mgr;
        m_conn_mgr = NULL;
    }
    return 0;
}
