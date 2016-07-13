#ifndef _CONNECTOR_MGR_H_
#define _CONNECTOR_MGR_H_
#include <list>
#include "connector.h"

const int kMaxEpollEvent = 1024;

class App;
class ConnectorMgr
{
public:
    ConnectorMgr();
    ~ConnectorMgr();
    int Init(App* processor);
    int Update();
    int Fini();
    void CloseConnectors();
private:
    int _epfd;
    EpollEvent* _events;
    std::list<Connector*> _access_list;
};
#endif

