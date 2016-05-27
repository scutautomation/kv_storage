#include <iostream>
#include <unistd.h>
#include "server_app.h"

int main(int argc, char* argv[])
{
    Envs envs;
    envs.is_daemon = true;
    envs.app_id = 0;
    envs.app_name = "proxy";
    ServerApp app(envs);
    return app.Start(argc, argv);
}
