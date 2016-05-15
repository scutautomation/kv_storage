#include "mempool.h"
#include <stdint.h>
#include "log.h"

MemPool::MemPool() : m_idxs(NULL), m_ptr(NULL), m_block_count(0),
                     m_block_size(0), m_idx_top(0), m_type(POOL_TYPE_8)
{

}

MemPool::~MemPool() {}

int MemPool::Init(int32_t block_count, MemPoolType pool_type)
{
    switch (pool_type)
    {
        case POOL_TYPE_8:
            m_block_size = BLOCK_8_SIZE;
            break;
        case POOL_TYPE_16:
            m_block_size = BLOCK_16_SIZE;
            break;
        case POOL_TYPE_32:
            m_block_size = BLOCK_32_SIZE;
            break;
        case POOL_TYPE_64:
            m_block_size = BLOCK_64_SIZE;
            break;
        default:
            LogError("pool type not correct");
            return -1;
    }
    m_type = pool_type;
    m_block_count = block_count;
    int32_t idxs_size =  (sizeof(int32_t) * block_count);
    int32_t body_size = m_block_size * m_block_count;

    // 内存池总大小为头部index区域+存数据的实际区域
    char* ptr = new char[idxs_size + body_size];
    LogInfo("create memory pool, start:%p, end:%p, block_count:%d, block_size:%d",
            ptr, ptr + idxs_size + body_size, m_block_count, m_block_size);
    if (!ptr)
    {
        LogError("failed to create memory pool");
        return -1;
    }
    m_ptr = ptr + idxs_size;
    m_idxs = reinterpret_cast<int32_t*>(ptr);
    InitIdxs();
    return 0;
}

void MemPool::Destroy()
{
    char* ptr = reinterpret_cast<char*>(m_idxs);
    if (ptr)
    {
        delete[] ptr;
        m_ptr = NULL;
        m_idxs = NULL;
        m_idx_top = 0;
        m_block_count = 0;
    }
}

int MemPool::PopIdx(int32_t* idx)
{
    if (m_idx_top < 0)
    {
        LogError("there is no index left in index area");
        return -1;
    }
    *idx = m_idxs[m_idx_top];
    --m_idx_top;
    return 0;
}

int MemPool::PushIdx(int32_t idx)
{
    if (idx < 0 || idx > (m_block_count - 1))
    {
        LogError("memory index %d invalid", idx);
        return -1;
    }
    if (m_idx_top >= m_block_size)
    {
        LogError("index area is full");
        return -1;
    }
    ++m_idx_top;
    m_idxs[m_idx_top] = idx;
    return 0;
}

void MemPool::InitIdxs()
{
    // 初始建立块索引
    for (int i = 0; i < m_block_count; ++i)
    {
        m_idxs[i] = i;
    }
    m_idx_top = m_block_count - 1;
}

int MemPool::Alloc(int32_t* idx)
{
    LogDebug("pool type is %d", m_type);
    int32_t idxx = 0;
    if (PopIdx(&idxx) != 0)
    {
        LogError("can not alloc memory");
        return -1;
    }
    *idx = idxx;
    return 0;
}

void* MemPool::Get(int32_t idx)
{
    LogDebug("pool type is %d", m_type);
    if (idx < 0 || idx >= m_block_count)
    {
        LogError("index %d is invalid, must bet between 0 and %d", idx, m_block_count);
        return NULL;
    }
    char* ptr = m_ptr + m_block_size * idx;
    return ptr;
}

int MemPool::Free(int32_t idx)
{
    LogDebug("pool type is %d", m_type);
    if (PushIdx(idx))
    {
        LogError("can not free memory");
        return -1;
    }
    return 0;
}

AbundantMemPool::AbundantMemPool() : m_block8_baseidx(0), m_block16_baseidx(0),
                                     m_block32_baseidx(0), m_block64_baseidx(0),
                                     m_maxidx(0)
{

}

AbundantMemPool::~AbundantMemPool()
{

}

#define CHECK_VALID(field, field_desc) \
    do \
    { \
        if (field < 0 || field > MAX_POOL_BLOCK_COUNT) \
        { \
            LogError("invalid field:" field_desc); \
            return false; \
        } \
    } while(0)

bool AbundantMemPool::IsConfValid(const AbundantMemPoolConf& conf)
{
    CHECK_VALID(conf.block8_count, "block8_count");
    CHECK_VALID(conf.block16_count, "block16_count");
    CHECK_VALID(conf.block32_count, "block32_count");
    CHECK_VALID(conf.block64_count, "block64_count");
    return true;
}

int AbundantMemPool::Init(const AbundantMemPoolConf& conf)
{
    if (!IsConfValid(conf))
    {
        LogError("memory pool configuration invalid");
        return -1;
    }
    int ret = 0;
    do
    {
        if (m_block8_pool.Init(conf.block8_count, POOL_TYPE_8) != 0)
        {
            LogError("create m_block8_pool failed");
            ret = -1;
            break;
        }
        m_block8_baseidx = 0;
        if (m_block16_pool.Init(conf.block16_count, POOL_TYPE_16) != 0)
        {
            ret = -1;
            LogError("create m_block8_pool failed");
            break;
        }
        m_block16_baseidx = conf.block8_count;
        if (m_block32_pool.Init(conf.block32_count, POOL_TYPE_32) != 0)
        {
            ret = -1;
            LogError("create m_block8_pool failed");
            break;
        }
        m_block32_baseidx = m_block16_baseidx + conf.block16_count;
        if (m_block64_pool.Init(conf.block64_count, POOL_TYPE_64) != 0)
        {
            ret = -1;
            LogError("create m_block8_pool failed");
            break;
        }
        m_block64_baseidx = m_block32_baseidx + conf.block32_count;
        m_maxidx = m_block64_baseidx + conf.block64_count;
    } while(0);
    LogDebug("block8_baseidx:%d, block16_baseidx:%d, block32_baseidx:%d, block64_baseidx:%d", 
             m_block8_baseidx, m_block16_baseidx, m_block32_baseidx, m_block64_baseidx);
    if (ret != 0)
    {
        m_block8_pool.Destroy();
        m_block16_pool.Destroy();
        m_block32_pool.Destroy();
        m_block64_pool.Destroy();
        return ret;
    }
    return 0;
}

void AbundantMemPool::Destroy()
{
    m_block8_pool.Destroy();
    m_block16_pool.Destroy();
    m_block32_pool.Destroy();
    m_block64_pool.Destroy();
}

int AbundantMemPool::Alloc(int32_t size, int32_t* idx)
{
    if (size < 0 || size > BLOCK_64_SIZE)
    {
        LogError("alloc size %d is invalid, must between 0 and %d", size, BLOCK_64_SIZE);
        return -1;
    }
    if (size == 0)
    {
        return m_block8_pool.Alloc(idx);
    }
    int choice = (size - 1) / BLOCK_8_SIZE;
    int32_t raw_idx = 0;
    int ret = 0;
    switch (choice)
    {
        case 0:
            return m_block8_pool.Alloc(idx);
        case 1:
            ret = m_block16_pool.Alloc(&raw_idx);
            *idx = m_block16_baseidx + raw_idx;
            LogDebug("idx:%d, raw_idx:%d", *idx, raw_idx);
            return ret;
        case 2:
        case 3:
            ret = m_block32_pool.Alloc(&raw_idx);
            *idx = m_block32_baseidx + raw_idx;
            LogDebug("idx:%d, raw_idx:%d", *idx, raw_idx);
            return ret;
        case 4:
        case 5:
        case 6:
        case 7:
            ret = m_block64_pool.Alloc(&raw_idx);
            *idx = m_block64_baseidx + raw_idx;
            LogDebug("idx:%d, raw_idx:%d", *idx, raw_idx);
            return ret;
        default:
            LogError("can not find match memory pool, can not alloc memory");
    }
    return -1;
}

#define MEMORY_POOL_OPERATION(method, err_rv) \
    do \
    { \
        if (idx < 0) \
        { \
            LogError("index %d is invalid", idx); \
            return err_rv; \
        } \
        int32_t real_idx = 0; \
        if (idx < m_block16_baseidx) \
        { \
            LogDebug("idx:%d, read_idx:%d", idx, real_idx); \
            return m_block8_pool.method(idx); \
        } \
        if (idx < m_block32_baseidx) \
        { \
            real_idx = idx - m_block16_baseidx; \
            LogDebug("idx:%d, read_idx:%d", idx, real_idx); \
            return m_block16_pool.method(real_idx); \
        } \
        if (idx < m_block64_baseidx) \
        { \
            real_idx = idx - m_block32_baseidx; \
            LogDebug("idx:%d, read_idx:%d", idx, real_idx); \
            return m_block32_pool.method(real_idx); \
        } \
        if (idx < m_maxidx) \
        { \
            real_idx = idx - m_block64_baseidx; \
            LogDebug("idx:%d, read_idx:%d", idx, real_idx); \
            return m_block64_pool.method(real_idx); \
        } \
        LogError("no match memory found, can not access memory by index %d", idx); \
    } while (0)

void* AbundantMemPool::Get(int32_t idx)
{
    MEMORY_POOL_OPERATION(Get, NULL);
    return NULL;
}

int AbundantMemPool::Free(int32_t idx)
{
    MEMORY_POOL_OPERATION(Free, -1);
    return 0;
}
