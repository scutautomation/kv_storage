#include "server_app.h"
#include <iostream>
#include <unistd.h>
#include "platform/log.h"

ServerApp::ServerApp(Envs envs) : App(envs)
{

}

ServerApp::~ServerApp()
{

}

int ServerApp::Init(const Envs& envs)
{
    LogInfo("server %d init", envs.app_id);
    return 0;
}

int ServerApp::Proc(const Envs& envs)
{
    LogInfo("server %d proc", envs.app_id);
    sleep(3);
    return 0;
}

int ServerApp::Tick(const Envs& envs)
{
    LogInfo("server %d tick", envs.app_id);
    return 0;
}

int ServerApp::Fini(const Envs& envs)
{
    LogInfo("server %d fini", envs.app_id);
    return 0;
}
