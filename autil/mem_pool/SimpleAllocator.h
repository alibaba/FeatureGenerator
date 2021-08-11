#pragma once

#include <stddef.h>
#include <new>
#include <memory>

#include "autil/mem_pool/ChunkAllocatorBase.h"

namespace autil { namespace mem_pool {

class SimpleAllocator : public ChunkAllocatorBase
{
public:
    SimpleAllocator();
    ~SimpleAllocator();

public:
    virtual void* doAllocate(size_t numBytes)
    {
        return static_cast<void*>(new (std::nothrow) char[numBytes]);
    }
    virtual void doDeallocate(void* const addr, size_t numBytes)
    {
        delete[] (char*)addr;
    }
};

typedef std::shared_ptr<SimpleAllocator> SimpleAllocatorPtr;

}}

