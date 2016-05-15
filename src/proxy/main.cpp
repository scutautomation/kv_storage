#include <iostream>
#include <unistd.h>
#include "proxy_app.h"

int main(int argc, char* argv[])
{
    Envs envs;
    envs.is_daemon = true;
    envs.app_id = 0;
    envs.app_name = "proxy";
    ProxyApp app(envs);
    return app.Start(argc, argv);
}
