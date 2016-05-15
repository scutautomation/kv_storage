#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_
#include <stdint.h>

#define BLOCK_8_SIZE (8 * 1024)
#define BLOCK_16_SIZE (16 * 1024)
#define BLOCK_32_SIZE (32 * 1024)
#define BLOCK_64_SIZE (64 * 1024)

// 每个内存池最多支持分配1024个块
#define MAX_POOL_BLOCK_COUNT (1024)

struct AbundantMemPoolConf
{
    int32_t block8_count;
    int32_t block16_count;
    int32_t block32_count;
    int32_t block64_count;
    AbundantMemPoolConf() : block8_count(0), block16_count(0), block32_count(0),
                    block64_count(0)
    {
    }
};

enum MemPoolType
{
    POOL_TYPE_8,
    POOL_TYPE_16,
    POOL_TYPE_32,
    POOL_TYPE_64
};

class MemPool
{
public:
    MemPool();
    ~MemPool();
    int Init(int32_t block_count, MemPoolType type);
    int Alloc(int32_t* idx);
    void* Get(int32_t idx);
    int Free(int32_t idx);
    void Destroy();
private:
    MemPool(const MemPool&);
    MemPool& operator= (const MemPool&);
    void InitIdxs();
    int PopIdx(int32_t* idx);
    int PushIdx(int32_t idx);

    int32_t* m_idxs;
    char* m_ptr;
    int32_t m_block_count;
    int32_t m_block_size;
    int32_t m_idx_top;
    MemPoolType m_type;
};

class AbundantMemPool
{
public:
    AbundantMemPool();
    ~AbundantMemPool();
    int Init(const AbundantMemPoolConf& conf);
    int Alloc(int32_t size, int32_t* idx);
    void* Get(int idx);
    int Free(int idx);
    void Destroy();
private:
    bool IsConfValid(const AbundantMemPoolConf& conf);
    MemPool m_block8_pool;
    int m_block8_baseidx;
    MemPool m_block16_pool;
    int m_block16_baseidx;
    MemPool m_block32_pool;
    int m_block32_baseidx;
    MemPool m_block64_pool;
    int m_block64_baseidx;
    int m_maxidx;
};

#endif
