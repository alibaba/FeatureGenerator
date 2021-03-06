#pragma once

#include <stdlib.h>
#include <mutex>

#include "autil/Log.h"
#include "autil/mem_pool/AllocatePolicy.h"
#include "autil/mem_pool/MemoryChunk.h"
#include "autil/mem_pool/PoolBase.h"

namespace autil { namespace mem_pool {
class ChunkAllocatorBase;

class Pool : public PoolBase
{
public:
    static const size_t DEFAULT_CHUNK_SIZE = 10 * 1024 * 1024; // 10M
    static const size_t DEFAULT_ALIGN_SIZE = sizeof(char*);
public:
    Pool(ChunkAllocatorBase* allocator, size_t chunkSize,
         size_t alignSize = DEFAULT_ALIGN_SIZE);

    Pool(size_t chunkSize = DEFAULT_CHUNK_SIZE,
         size_t alignSize = DEFAULT_ALIGN_SIZE);

    virtual ~Pool();

private:
    Pool(const Pool&);
    void operator = (const Pool&);

public:
    /**
     * Allocate memory from Pool
     * @param numBytes number of bytes need to allocate
     * @return address of allocated memory
     */
    void* allocate(size_t numBytes) override;

    void* allocateUnsafe(size_t numBytes);

    /**
     * Free memory, dummy function
     */
    void deallocate(void*, size_t) override {}

    /**
     * release all allocated memory in Pool
     */
    void release() override;

    /**
     * Reset memory to init state
     */
    size_t reset() override;
    size_t resetUnsafe();

    /**
     * clear pool data and reset
     */
    void clear();

    void *allocateAlign(size_t numBytes, size_t alignSize);
    /**
     * get alignment size
     * @return alignSize alignment size
     */
    size_t getAlignSize() const {return _alignSize;}

    /**
     * Get used size in bytes
     */
    size_t getUsedBytes() const;

    /**
     * Get total size in bytes
     */
    size_t getTotalBytes() const;

    /**
     *Get allocated size from pool
     */
    size_t getAllocatedSize() const;

    size_t getAvailableChunkSize() const;

    /*
     * Check whether pointer in pool
     */
    bool isInPool(const void *ptr) const override;

    static size_t alignBytes(size_t numBytes, size_t alignSize);

protected:
    mutable std::mutex _mutex;
    size_t _alignSize; ///Alignment length
    MemoryChunk* _memChunk;  ///Memory chunk
    size_t _allocSize;// size of allocated memory from this pool

    AllocatePolicy* _allocPolicy;///allocate policy
    static MemoryChunk DUMMY_CHUNK;///dummy chunk
private:
    AUTIL_LOG_DECLARE();
};

typedef std::shared_ptr<Pool> PoolPtr;

class UnsafePool : public Pool {
public:
    UnsafePool(ChunkAllocatorBase* allocator, size_t chunkSize,
         size_t alignSize = DEFAULT_ALIGN_SIZE);

    UnsafePool(size_t chunkSize = DEFAULT_CHUNK_SIZE,
               size_t alignSize = DEFAULT_ALIGN_SIZE);
private:
    UnsafePool(const UnsafePool&);
    void operator = (const UnsafePool&);
public:
    void* allocate(size_t numBytes) override {
        return allocateUnsafe(numBytes);
    }
    size_t reset() override {
        return resetUnsafe();
    }
};

//////////////////////////////////////////////////
//implementation

inline void *Pool::allocateAlign(size_t numBytes, size_t alignSize) {
    numBytes += alignSize;
    static_assert(sizeof(size_t) == sizeof(void*), "size not equal");
    return (void*)alignBytes((size_t)allocate(numBytes), alignSize);
}

inline void Pool::release()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _allocPolicy->release();
    _allocSize = 0;
    _memChunk = &(Pool::DUMMY_CHUNK);
}

inline size_t Pool::reset()
{
    std::unique_lock<std::mutex> lock(_mutex);
    return resetUnsafe();
}

inline size_t Pool::resetUnsafe() {
    size_t totalSize = _allocPolicy->reset();
    _memChunk = &(Pool::DUMMY_CHUNK);
    _allocSize = 0;
    return totalSize;
}

inline size_t Pool::getUsedBytes() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    size_t allocBytes = _allocPolicy->getUsedBytes();
    allocBytes -= _memChunk->getFreeSize();
    return allocBytes;
}

inline size_t Pool::getAllocatedSize() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _allocSize;
}

inline size_t Pool::getTotalBytes() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _allocPolicy->getTotalBytes();
}

inline size_t Pool::getAvailableChunkSize() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _allocPolicy->getAvailableChunkSize();
}

inline size_t Pool::alignBytes(size_t numBytes, size_t alignSize)
{
    return ((numBytes + alignSize - 1) & ~(alignSize - 1));
}

}}

