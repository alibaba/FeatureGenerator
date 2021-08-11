#include "autil/mem_pool/SimpleAllocatePolicy.h"

#include <cstddef>

#include "autil/mem_pool/SimpleAllocator.h"

using namespace std;

namespace autil { namespace mem_pool {
AUTIL_DECLARE_AND_SETUP_LOGGER(autil, SimpleAllocatePolicy);

SimpleAllocatePolicy::SimpleAllocatePolicy(ChunkAllocatorBase* allocator, 
        size_t chunkSize)
    : _allocator(allocator)
    , _ownAllocator(false)
    , _chunkHeader(NULL)
    , _currentChunk(NULL)
    , _chunkSize(chunkSize)
    , _usedBytes(0)
    , _totalBytes(0)
{
}

SimpleAllocatePolicy::SimpleAllocatePolicy(size_t chunkSize)
    : _allocator(new SimpleAllocator())
    , _ownAllocator(true)
    , _chunkHeader(NULL)
    , _currentChunk(NULL)
    , _chunkSize(chunkSize)
    , _usedBytes(0)
    , _totalBytes(0)
{
}

SimpleAllocatePolicy::~SimpleAllocatePolicy()
{
    release();

    if (_ownAllocator)
    {
        delete _allocator;
    }
    _allocator = NULL;
}

}}

