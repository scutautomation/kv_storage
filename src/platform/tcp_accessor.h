#ifndef _TCP_CONNECTOR_H_
#define _TCP_CONNECTOR_H_
#include "net_interface.h"

class App;
class TcpAccessor : public NetAccessor
{
public:
    TcpAccessor();
    ~TcpAccessor();
    int Init(int sockfd, int epfd, App* processor);
    int Update();
    int Fini();
    int Send(void* buf, int buf_len);
    int Recv();
private:
    void FiniSock();

    int m_sockfd;
    int m_epfd;
    App* m_processor;
    EventBuffer m_recv_buffer;
    EventBuffer m_send_buffer;
    ConnHead m_conn_head;
};
#endif
