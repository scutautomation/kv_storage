#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_
#include "common.h"

class App;
class Connector
{
public:
    Connector();
    virtual ~Connector();
    virtual int Init(int epfd, App* app) = 0;
    virtual int Update() = 0;
    virtual int Fini() = 0;
    virtual int Send(void* buf, int buf_len) = 0;
    virtual int Recv() = 0;
    virtual int GetStatus()
    {
        return _status;
    }

    void SetEvents(int event)
    {
        _events |= event;
    }
protected:
    int _events;
    int _status;
};
#endif

