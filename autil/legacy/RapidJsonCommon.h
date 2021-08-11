#pragma once

#include <assert.h>
#include <string.h>
#include <iosfwd>

#include "autil/mem_pool/Pool.h"

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/rapidjson.h"

namespace autil {

class AutilPoolAllocator {
public:
    static const bool kNeedFree = false;

    AutilPoolAllocator(autil::mem_pool::Pool *pool_ = NULL)
    {
        if (pool_) {
            pool = pool_;
            ownPool = false;
        } else {
            pool = new autil::mem_pool::Pool;
            ownPool = true;
        }
    }
    ~AutilPoolAllocator() {
        if (ownPool) {
            DELETE_AND_SET_NULL(pool);
        }
    }
private:
    AutilPoolAllocator(const AutilPoolAllocator &);
    AutilPoolAllocator& operator=(const AutilPoolAllocator &);

public:
    void* Malloc(size_t size) {
        if (size == 0) {
            return NULL;
        } else {
            return pool->allocate(size);
        }
    }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        // do nothing to old mem
        if (originalPtr == 0) {
            return Malloc(newSize);
        }
        if (newSize == 0) {
            return NULL;
        }
        if (originalSize >= newSize) {
            return originalPtr;
        }
        void* newBuffer = Malloc(newSize);
        assert(newBuffer != 0);
        if (originalSize)
            std::memcpy(newBuffer, originalPtr, originalSize);
        return newBuffer;
    }
    static void Free(void *ptr) {
        // do nothing to old mem
    }
public:
    autil::mem_pool::Pool *pool;
    bool ownPool;
};

typedef rapidjson::GenericValue<rapidjson::ASCII<>, AutilPoolAllocator> SimpleValue;
typedef rapidjson::GenericDocument<rapidjson::ASCII<>, AutilPoolAllocator> SimpleDocument;

}
