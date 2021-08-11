#include "autil/mem_pool/Pool.h"

#include <stdint.h>
#include "autil/Log.h"
#include "autil/mem_pool/SimpleAllocatePolicy.h"

namespace autil { namespace mem_pool {
class ChunkAllocatorBase;

AUTIL_LOG_SETUP(autil, Pool);

MemoryChunk Pool::DUMMY_CHUNK;

Pool::Pool(ChunkAllocatorBase* allocator, size_t chunkSize, size_t alignSize)
    : _alignSize(alignSize)
    , _memChunk(&(Pool::DUMMY_CHUNK))
    , _allocSize(0)
    , _allocPolicy(new SimpleAllocatePolicy(allocator, chunkSize))
{
}

Pool::Pool(size_t chunkSize, size_t alignSize)
    : _alignSize(alignSize)
    , _memChunk(&(Pool::DUMMY_CHUNK))
    , _allocSize(0)
    , _allocPolicy(new SimpleAllocatePolicy(chunkSize))
{
}

Pool::~Pool()
{
    _memChunk = NULL;
    delete _allocPolicy;
    _allocPolicy = NULL;
}

bool Pool::isInPool(const void *ptr) const {
    return _allocPolicy->isInChunk(ptr);
}

void* Pool::allocate(size_t numBytes)
{
    ScopedSpinLock lock(_mutex);
    return allocateUnsafe(numBytes);
}

void* Pool::allocateUnsafe(size_t numBytes) {
    size_t allocSize = alignBytes(numBytes, _alignSize);
    void* ptr = _memChunk->allocate(allocSize);
    if(!ptr)
    {
        MemoryChunk* chunk = _allocPolicy->allocate(allocSize);
        if (!chunk)
        {
            AUTIL_LOG(ERROR, "Allocate too large memory chunk: %ld, "
                   "max available chunk size: %ld",
                   (int64_t)numBytes, (int64_t)_allocPolicy->getAvailableChunkSize());
            return NULL;
        }
        _memChunk = chunk;
        ptr = _memChunk->allocate(allocSize);
    }
    _allocSize += allocSize;
    return ptr;
}

void Pool::clear() {
    ScopedSpinLock lock(_mutex);
    _allocPolicy->clear();
}

// UnsafePool
UnsafePool::UnsafePool(ChunkAllocatorBase* allocator, size_t chunkSize,
                       size_t alignSize)
    : Pool(allocator, chunkSize, alignSize)
{
}

UnsafePool::UnsafePool(size_t chunkSize, size_t alignSize)
    : Pool(chunkSize, alignSize)
{
}

}}
