#include "autil/mem_pool/RecyclePool.h"

#include <iosfwd>


namespace autil {
namespace mem_pool {
class ChunkAllocatorBase;
}  // namespace mem_pool
}  // namespace autil

using namespace std;
using namespace autil;

namespace autil { namespace mem_pool {
AUTIL_LOG_SETUP(autil, RecyclePool);

RecyclePool::RecyclePool(ChunkAllocatorBase* allocator, size_t chunkSize,
                         size_t alignSize)
    : Pool(allocator, chunkSize, alignSize)
    , _freeSize(0)
{
}

RecyclePool::RecyclePool(size_t chunkSize, size_t alignSize)
    : Pool(chunkSize, alignSize)
    , _freeSize(0)
{
}

RecyclePool::~RecyclePool() 
{
}

}}

