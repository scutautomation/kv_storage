#include "net_interface.h"
#include <fcntl.h>
#include "log.h"

void SetNonBlocking(int fd)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        LogError("get fcntl failed");
        return;
    }
    opts |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        LogError("set fcntl O_NONBLOCK failed");
        return;
    }
}

Connector::Connector() : m_events(0), m_status(CONNECTOR_STAUTS_IDLE)
{}

Connector::~Connector() {}