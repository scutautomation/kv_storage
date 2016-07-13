#ifndef _SERVER_H_
#define _SERVER_H_
#include "platform/app.h"
#include "platform/connector_mgr.h"

class ServerApp : public App
{
public:
    ServerApp(Envs envs);
    ~ServerApp();

    int Init(const Envs& envs);
    int Proc(const Envs& envs);
    int Tick(const Envs& envs);
    int Fini(const Envs& envs);
    int OnRecvClient(ConnHead conn_head, void* buf, int32_t buf_len);
private:
    ConnectorMgr* _conn_mgr;
};
#endif
