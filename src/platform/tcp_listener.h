#ifndef _TCP_LISTENER_H_
#define _TCP_LISTENER_H_
#include "connector.h"
#include <list>

class TcpListener : public Connector
{
public:
    TcpListener();
    ~TcpListener();
    int Init(int epfd, App* processor);
    int Update();
    int Fini();
    int Send(void* buf, int buf_len);
    int Recv();
private:
    int _sockfd;
    int _epfd;
    std::list<Connector*> _connector_list;
    App* _processor;
};
#endif

