#ifndef _CONNECTOR_MGR_H_
#define _CONNECTOR_MGR_H_
#include <list>
#include "common.h"

const int kMaxEpollEvent = 1024;

class App;
class NetAccessor;
class NetListener;
class TcpListener;
class TcpAccessor;

class ConnectorMgr
{
public:
    ConnectorMgr();
    ~ConnectorMgr();
    int Init(App* processor);
    int Update();
    int Fini();
    TcpListener* CreateTcpListener();
    TcpAccessor* CreateTcpAccessor(int fd);
    void OnNewTcpConnect(int fd);
    int GetConnectionCount();
private:
    int m_epfd;
    EpollEvent* m_events;
    App* m_processor;
    std::list<NetAccessor*> m_access_list;
    std::list<NetListener*> m_listener_list;
};
#endif

