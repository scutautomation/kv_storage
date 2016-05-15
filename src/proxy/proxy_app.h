#ifndef _PROXY_H_
#define _PROXY_H_
#include "platform/app.h"
class ProxyApp : public App
{
public:
    ProxyApp(Envs envs);
    ~ProxyApp();

    int Init(const Envs& envs);
    int Proc(const Envs& envs);
    int Tick(const Envs& envs);
    int Fini(const Envs& envs);
};
#endif
