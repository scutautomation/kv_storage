#ifndef _SERVER_H_
#define _SERVER_H_
#include "platform/app.h"
#include "platform/epoller.h"

class ServerApp : public App
{
public:
    ServerApp(Envs envs);
    ~ServerApp();

    int Init(const Envs& envs);
    int Proc(const Envs& envs);
    int Tick(const Envs& envs);
    int Fini(const Envs& envs);
private:
    Epoller* _epoller;
    Acceptor* _acceptor;
};
#endif
