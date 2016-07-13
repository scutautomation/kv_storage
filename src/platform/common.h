#ifndef _KV_COMMON_H_
#define _KV_COMMON_H_
enum ConnectorStatus
{
    CONNECTOR_STAUTS_IDLE,
    CONNECTOR_STATUS_CONNECTED,
    CONNECTOR_STATUS_DISCONNECTED
};

struct ConnHead
{
    void* ptr;
};

const int kMaxEventBufferLen = 10 * 1024;
struct EventBuffer
{
    char* buf;
    int buf_len;
    int data_len;
    int start_pos;
    int end_pos;
};

typedef struct epoll_event EpollEvent;
#endif
