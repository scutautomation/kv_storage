#include "proxy_app.h"
#include <iostream>
#include "platform/log.h"

ProxyApp::ProxyApp(Envs envs) : App(envs)
{

}

ProxyApp::~ProxyApp()
{

}

int ProxyApp::Init(const Envs& envs)
{
    LogInfo("proxy %d init", envs.app_id);
    return 0;
}

int ProxyApp::Proc(const Envs& envs)
{
    LogInfo("proxy %d proc", envs.app_id);
    return 0;
}

int ProxyApp::Tick(const Envs& envs)
{
    LogInfo("proxy %d tick", envs.app_id);
    return 0;
}

int ProxyApp::Fini(const Envs& envs)
{
    LogInfo("proxy %d fini", envs.app_id);
    return 0;
}
