#include "app.h"
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <iostream>
#include "log.h"

#define LOG_PATH "log"
#ifndef LOG_BASE_NAME
    #define LOG_BASE_NAME "app.log"
#endif
#ifndef LOG_LEVEL
    #define LOG_LEVEL INFO
#endif

bool App::m_running = false;
int App::Start(int argc, char* argv[])
{
    if (LogInit(GlobalLog(), LOG_PATH, LOG_BASE_NAME))
    {
        std::cout<<"log init failed, app start failed"<<std::endl;
        return -1;
    }
    LogConf conf;
    conf.level = LOG_LEVEL;
    LogRefresh(GlobalLog(), conf);
    if (InternalInit(argc, argv) != 0)
    {
        LogError("InternalInit error, start app failed");
        return -1;
    }
    if (Init(m_envs) != 0)
    {
        LogError("Init error, start app failed");
        return -1;
    }
    Loop();
    Fini(m_envs);
    LogInfo("app exited");
    LogFini(GlobalLog());
    return 0;
}

void App::Stop()
{
    m_running = false;
    LogInfo("app exiting");
}

void App::SigHandler(int sig)
{
    if (sig == SIGTERM)
    {
        Stop();
    }
}

void App::SetSignal()
{
    // 默认信号处理
    struct sigaction sa;
    sa.sa_handler = &App::SigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    SelfDefSignal();
}

void App::MakeDaemon()
{
    umask(0);
    int pid = fork();
    if (pid < 0)
    {
        LogError("create daemon failed");
        exit(-1);
    }
    if (pid > 0)
    {
        exit(0);
    }
    setsid();
    struct rlimit limit;
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0)
    {
        LogError("getrlimit failed");
        exit(-1);
    }
    for (int i = 0; i < limit.rlim_max; ++i)
    {
        close(i);
    }
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, NULL);
    pid = fork();
    if (pid < 0)
    {
        LogError("create daemon failed");        
        exit(-1);
    }
    if (pid > 0)
    {
        exit(0);
    }
}

int App::InternalInit(int argc, char* argv[])
{
    if (m_envs.is_daemon)
    {
        MakeDaemon();
    }
}

int App::Loop()
{
    m_running = true;
    while (m_running)
    {
        int ret = Proc(m_envs);
        if (ret != 0)
        {
            usleep(m_envs.sleep_time);
            continue;
        }
    }
    return 0;
}
