#include <iostream>
#include "mempool.h"
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

struct TestStruct
{
    int a;
    int b;
    int c;
};

int main()
{
    if (LogInit(GlobalLog(), "./log", "app.log"))
    {
        std::cout<<"log init failed, app start failed"<<std::endl;
        return -1;
    }
    LogConf conf;
    conf.level = DEBUG;
    LogRefresh(GlobalLog(), conf);
    /*
    MemPool pool;
    int ret = pool.Init(10, POOL_TYPE_8);
    if (ret != 0)
    {
        std::cout<<"pool init failed"<<std::endl;
        return -1;
    }
    int32_t idx;
    int32_t idx1;
    int32_t idx2;
    char* ptr = NULL;
    for (int i = 0; i < 20; ++i)
    {
        ret = pool.Alloc(&idx);
        if (ret != 0)
        {
            std::cout<<"alloc memory failed"<<std::endl;
            return -1;
        }
        ptr = (char*) pool.Get(idx);
        TestStruct* tst = (TestStruct*)ptr;
        tst->a = 3;
        tst->b = 4;
        tst->c = 5;
        printf("%s, %d, index:%d, ptr:%p, a:%d, b:%d, c:%d\n", __FILE__, __LINE__,
                idx, ptr, tst->a,tst->b, tst->c);
        snprintf(ptr, BLOCK_8_SIZE, "%s", "hello world");
        if (i % 2 == 0)
        {
            printf("i:%d, free %d\n", i, idx);
            pool.Free(idx);
        }
    }
    ret = pool.Alloc(&idx1);
    if (ret != 0)
    {
        std::cout<<"alloc memory failed"<<std::endl;
        return -1;
    }
    ptr = (char*) pool.Get(idx1);
    printf("%s, %d, index:%d, addr:%p\n", __FILE__, __LINE__,idx1, ptr);
    snprintf(ptr, BLOCK_8_SIZE, "%s", "hello world");

    if (pool.Free(idx))
    {
        printf("free failed\n");
    }

    ret = pool.Alloc(&idx2);
    if (ret != 0)
    {
        std::cout<<"alloc memory failed"<<std::endl;
        return -1;
    }
    ptr = (char*) pool.Get(idx2);
    printf("%s, %d, index:%d, addr:%p\n", __FILE__, __LINE__,idx2, ptr);
    snprintf(ptr, BLOCK_8_SIZE, "%s", "hello world");

    if (pool.Free(idx1))
    {
        printf("free failed\n");
    }
    if (pool.Free(idx2))
    {
        printf("free failed\n");
    }

    pool.Destroy();
    */
    AbundantMemPool pool;
    AbundantMemPoolConf poolconf;
    poolconf.block8_count = 3;
    poolconf.block16_count = 1;
    poolconf.block32_count = 10;
    poolconf.block64_count = 1;
    pool.Init(poolconf);
    int32_t idx = 0;
    int ret = pool.Alloc(BLOCK_8_SIZE, &idx);
    char* ptr = (char*)pool.Get(idx);
    printf("%s, %d, index:%d, addr:%p\n", __FILE__, __LINE__,idx, ptr);

    int32_t idx1 = 0;
    ret = pool.Alloc(BLOCK_16_SIZE + 10, &idx1);
    char* ptr1 = (char*)pool.Get(idx1);
    snprintf(ptr1, BLOCK_32_SIZE, "%s", "hello world");
    printf("%s, %d, index:%d, addr:%p\n", __FILE__, __LINE__,idx1, ptr1);

    int32_t idx2 = 0;
    ret = pool.Alloc(BLOCK_32_SIZE + 10, &idx2);
    char* ptr2 = (char*)pool.Get(idx2);
    printf("%s, %d, index:%d, addr:%p\n", __FILE__, __LINE__,idx2, ptr2);

    pool.Free(idx);
    ret = pool.Alloc(BLOCK_8_SIZE + 10, &idx);
    char* ptr3 = (char*)pool.Get(idx);
    printf("%s, %d, index:%d, addr:%p\n", __FILE__, __LINE__,idx, ptr3);

    pool.Destroy();
    LogFini(GlobalLog());
    return 0;
}