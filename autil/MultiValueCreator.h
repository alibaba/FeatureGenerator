#pragma once

#include <assert.h>
#include <stddef.h>
#include <string>
#include <type_traits>
#include <vector>

#include "autil/MultiValueFormatter.h"
#include "autil/CountedMultiValueType.h"
#include "autil/mem_pool/Pool.h"

namespace autil {
class ConstString;


class MultiValueCreator
{
public:
    MultiValueCreator() {};
    ~MultiValueCreator() {};
private:
    MultiValueCreator(const MultiValueCreator &);
    MultiValueCreator& operator = (const MultiValueCreator &);
public:
    static char* createMultiStringBuffer(const std::vector<std::string>& strVec,
                                         mem_pool::PoolBase* pool = NULL);

    static char* createMultiStringBuffer(const std::vector<ConstString>& strVec,
                                         mem_pool::PoolBase* pool = NULL);

    template<typename T>
    static MultiValueType<T> convertToMultiValue(CountedMultiValueType<T> &value, mem_pool::PoolBase *pool = NULL);

    template <typename T>
    static typename std::enable_if<MULTI_VALUE_FORMAT_SUPPORTED(T), char*>::type
    createMultiValueBuffer(const std::vector<T>& values,
            mem_pool::PoolBase* pool = NULL);
    static char *createMultiValueBuffer(const std::vector<std::string> &data,
            mem_pool::PoolBase* pool = NULL);
    static char *createMultiValueBuffer(const std::vector<ConstString> &data,
            mem_pool::PoolBase* pool = NULL);

    template <typename T>
    static typename std::enable_if<MULTI_VALUE_FORMAT_SUPPORTED(T), char*>::type
    createMultiValueBuffer(const T* data, size_t size,
                           mem_pool::PoolBase* pool = NULL);
    static char *createMultiValueBuffer(const std::string* data, size_t size,
            mem_pool::PoolBase* pool = NULL);
    static char *createMultiValueBuffer(const ConstString* data, size_t size,
            mem_pool::PoolBase* pool = NULL);

    static char* createNullMultiValueBuffer(mem_pool::PoolBase* pool = NULL);
};

template<typename T>
inline MultiValueType<T> MultiValueCreator::convertToMultiValue(CountedMultiValueType<T> &value, mem_pool::PoolBase *pool) {
    if (!pool) {
        return MultiValueType<T>();
    }
    char* buffer = NULL;
    if (value.isNull()) {
        buffer = createNullMultiValueBuffer(pool);
    } else if (value.size() != 0) {
        buffer = createMultiValueBuffer(
                value.data(), (size_t)value.size(), pool);
    }
    MultiValueType<T> mt(buffer);
    return mt;
}

template <typename T>
inline typename std::enable_if<MULTI_VALUE_FORMAT_SUPPORTED(T), char*>::type
MultiValueCreator::createMultiValueBuffer(
        const std::vector<T>& values,
        mem_pool::PoolBase* pool)
{
    return createMultiValueBuffer(values.data(), values.size(), pool);
}

inline char* MultiValueCreator::createMultiValueBuffer(
        const std::vector<std::string>& values,
        mem_pool::PoolBase* pool)
{
    return createMultiValueBuffer(values.data(), values.size(), pool);
}

inline char* MultiValueCreator::createMultiValueBuffer(
        const std::vector<ConstString>& values,
        mem_pool::PoolBase* pool)
{
    return createMultiValueBuffer(values.data(), values.size(), pool);
}

template <typename T>
inline typename std::enable_if<MULTI_VALUE_FORMAT_SUPPORTED(T), char*>::type
MultiValueCreator::createMultiValueBuffer(const T* data, size_t size,
        mem_pool::PoolBase* pool)
{
    size_t bufLen = MultiValueFormatter::calculateBufferLen(size, sizeof(T));
    char* buffer = POOL_COMPATIBLE_NEW_VECTOR(pool, char, bufLen);
    assert(buffer);
    MultiValueFormatter::formatToBuffer<T>(data, size, buffer, bufLen);
    return buffer;
}

}
