#include "app.h"
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <sstream>
#include "connector.h"
#include "log.h"

#define LOG_PATH "log"
#ifndef LOG_BASE_NAME
    #define LOG_BASE_NAME "app.log"
#endif
#ifndef LOG_LEVEL
    #define LOG_LEVEL DEBUG
#endif

bool App::m_running = false;
int App::Start(int argc, char* argv[])
{
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
    MainLoop();
    Fini(m_envs);
    LogInfo("app exited");
    LogFini(GlobalLog());
    close(m_pidfd);
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
        LogInfo("catch SIGTERM");
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
    close(0);
    for (int i = 3; i < limit.rlim_max; ++i)
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
    if (LogInit(GlobalLog(), LOG_PATH, LOG_BASE_NAME))
    {
        std::cout<<"log init failed, app start failed"<<std::endl;
        return -1;
    }
    LogConf conf;
    conf.level = LOG_LEVEL;
    LogRefresh(GlobalLog(), conf);    
    LogInfo("start app %s", m_envs.app_name.c_str());
    SetSignal();
    return MakePidFile();
}

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
int App::MakePidFile()
{
    if (m_envs.app_name == "")
    {
        LogError("app_name can not be nil");
        return -1;
    }
    std::ostringstream oss;
    oss.str("");
    oss<<"/tmp/"<<m_envs.app_name<< "_"<<m_envs.app_id<<".pid";
    std::string pidfile = oss.str();
    int m_pidfd = open(pidfile.c_str(), O_RDWR|O_CREAT, LOCKMODE);
    if (m_pidfd < 0)
    {
        LogError("can not create pidfile %s", pidfile.c_str());
        return -1;
    }
    LogInfo("create pid file %s", pidfile.c_str());
    int ret = LockFile(m_pidfd);
    if (ret < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            std::cout<<"\nprogram is already running"<<std::endl;
            LogError("program is already running");
            close(m_pidfd);
            return -1;
        }
        std::cout<<"can not lock pidfile"<<pidfile.c_str()<<std::endl;
        LogError("can not lock pidfile %s", pidfile.c_str());
        close(m_pidfd);
        return -1;
    }
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%ld", (long)getpid());
    write(m_pidfd, buf, len);
    return 0;
}

int App::LockFile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}

int App::MainLoop()
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

void App::SendToClient(ConnHead conn_head, void* buf, int32_t buf_len)
{
    Connector* conn = (Connector*)conn_head.ptr;
    if (conn)
    {
        conn->Send(buf, buf_len);
    }
}
