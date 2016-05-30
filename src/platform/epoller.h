#ifndef _EPOLL_H_
#define _EPOLL_H_

static const int kMaxEpollEvent = 4096;
typedef struct epoll_event EpollEvent;
class EventBuffer
{
public:
    EventBuffer() {}
    virtual ~EventBuffer() {}
    int Push() {}
    int Pop() {}
};

class Epoller;

class Acceptor
{
public:
    Acceptor(int fd, EventBuffer* read_buf, EventBuffer* write_buf);
    virtual ~Acceptor();
    int OnCanRead(EpollEvent ev);
    int OnCanWrite(EpollEvent ev);
    void SetEpoller(Epoller* epoller)
    {
        _epoller = epoller;
    }

private:
    int Accept();
    EventBuffer* _read_buf;
    EventBuffer* _write_buf;
    int _fd;
    Epoller* _epoller;
};

class Epoller
{
public:
    Epoller(Acceptor* acceptor);
    virtual ~Epoller();
    int Create();
    void Destroy();
    int Dispatch();
    int Add(int fd, EpollEvent ev);
    int Mod(int fd, EpollEvent ev);
    int Del(int fd, EpollEvent ev);
private:
    int _epfd;
    EpollEvent* _events;
    Acceptor* _acceptor;
};
#endif
