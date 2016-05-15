#ifndef _KV_APP_H_
#define _KV_APP_H_
#include <stdint.h>
#include <string>

struct Envs
{
    uint32_t sleep_time;
    bool is_daemon;
    uint32_t app_id;
    std::string app_name;
};

class App
{
public:
    App(Envs envs) : m_envs(envs), m_pidfd(0) {}
    virtual ~App() {}

    // 启动函数，子类不需要重写或重载
    int Start(int argc, char* argv[]);
    
    // 启动时初始化函数，子类需要实现
    virtual int Init(const Envs& envs) = 0;

    // 主循环中循环调用的函数，子类需要实现
    virtual int Proc(const Envs& envs) = 0;

    // 定时调用函数，子类需要实现
    virtual int Tick(const Envs& envs) = 0;

    // 退出时调用的函数，子类需要实现
    virtual int Fini(const Envs& envs) = 0;

    // 自定义信号处理函数，子类可以不实现
    virtual void SelfDefSignal() {}
private:
    // 主循环，不断调用Proc
    int MainLoop();

    // 内部初始化：参数解析，创建守护进程，绑定pid文件等
    int InternalInit(int argc, char* argv[]);

    // 设置默认信号处理
    void SetSignal();
    
    // 创建守护进程
    void MakeDaemon();

    // 创建pid文件保证单实例运行
    int MakePidFile();

    static void SigHandler(int sig);

    // 停止进程（收到TERM信号）调用的函数
    static void Stop();

    // 锁住文件，用于锁住pid文件保证单实例运行
    int LockFile(int fd);

    Envs m_envs;
    static bool m_running;
    int m_pidfd;
};
#endif
