#ifndef _TCP_LISTENER_H_
#define _TCP_LISTENER_H_
#include "net_interface.h"
#include <list>

class TcpListener : public NetListener
{
public:
    TcpListener();
    ~TcpListener();
    int Init(int epfd, ConnectorMgr* conn_mgr);
    int Update();
    int Fini();
    int Accept();
private:
    int m_sockfd;
    int m_epfd;
    std::list<Connector*> _connector_list;
    ConnectorMgr* m_conn_mgr;
};
#endif

