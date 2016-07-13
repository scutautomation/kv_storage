#include <iostream>
#include <unistd.h>
#include "server_app.h"

int main(int argc, char* argv[])
{
    Envs envs;
    envs.is_daemon = false;
    envs.app_id = 0;
    envs.app_name = "server";
    envs.sleep_time = 200;
    ServerApp app(envs);
    return app.Start(argc, argv);
}
