#include <iostream>
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    LogConf conf;
    conf.level = DEBUG;
    LogInit(GlobalLog(), "./mylogdir", "app.log");
    LogRefresh(GlobalLog(), conf);
    int count = atoi(argv[1]);
    time_t t_start, t_end;
    t_start = time(NULL);
    for (int i = 0; i < count; ++i)
    {
        LogDebug("%s", "helloworld");
    }
    t_end = time(NULL);
    printf("time: %.6f s\n", difftime(t_end,t_start)) ;
    LogFini(GlobalLog());
    return 0;
}