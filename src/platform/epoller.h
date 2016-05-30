#ifndef _EPOLL_H_
#define _EPOLL_H_

static const int kMaxEpollEvent = 4096;
struct EpollEvent
{
    int fd;
    void* callback;
    int events;
};

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

    void SetRead(EventListener* listener)
    {
        _listener = listener;
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
    void Destory(); 
    int Dispatch();
    int Add(int fd, EpollEvent ev);
    int Mod(int fd, EpollEvent ev);
    int Del(int fd);
private:
    struct epoll_event EventTranslate(EpollEvent ev);
    int _epfd;
    struct epoll_event* _events;
    Acceptor* _acceptor;
};
#endif
