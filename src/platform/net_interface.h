#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_
#include "common.h"

void SetNonBlocking(int fd);
class App;
class Connector
{
public:
    Connector();
    virtual ~Connector();
    virtual int Init(int sockfd, int epfd, App* processor) = 0;
    virtual int Update() = 0;
    virtual int Fini() = 0;
    virtual int Send(void* buf, int buf_len) = 0;
    virtual int Recv() = 0;
    virtual int GetStatus()
    {
        return m_status;
    }

    void SetEvents(int event)
    {
        m_events |= event;
    }
protected:
    int m_events;
    int m_status;
};

class ConnectorMgr;
class NetListener : public Connector
{
public:
    NetListener() {}
    virtual ~NetListener() {}
    virtual int Init(int sockfd, int epfd, App* processor) { return 0; }
    virtual int Init(int epfd, ConnectorMgr* conn_mgr) = 0;
    virtual int Update() = 0;
    virtual int Fini() = 0;
    virtual int Accept() = 0;
    virtual int Send(void* buf, int buf_len) { return 0; }
    virtual int Recv() { return 0; }
};

class NetAccessor : public Connector
{
public:
    NetAccessor() {}
    virtual ~NetAccessor() {}
    virtual int Init(int sockfd, int epfd, App* processor) = 0;
    virtual int Update() = 0;
    virtual int Fini() = 0;
    virtual int Send(void* buf, int buf_len) = 0;
    virtual int Recv() = 0;
};
#endif

