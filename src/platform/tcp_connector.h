#ifndef _TCP_CONNECTOR_H_
#define _TCP_CONNECTOR_H_
#include "connector.h"

class App;
class TcpConnector : public Connector
{
public:
    TcpConnector();
    ~TcpConnector();
    int Init(int sockfd, App* processor);
    int Init(int sockfd, int epfd, App* processor);
    int Update();
    int Fini();
    int Send(void* buf, int buf_len);
    int Recv();
private:
    void FiniSock();

    int _sockfd;
    int _epfd;
    App* _processor;
    EventBuffer _recv_buffer;
    EventBuffer _send_buffer;
    ConnHead _conn_head;
};
#endif
