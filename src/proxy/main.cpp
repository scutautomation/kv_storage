#include <iostream>
#include "proxy_app.h"
int main(int argc, char* argv[])
{
    Envs envs;
    envs.is_daemon = false;
    envs.app_id = 0;
    ProxyApp app(envs);
    return app.Start(argc, argv);
}
