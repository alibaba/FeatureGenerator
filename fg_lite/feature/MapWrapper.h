#ifndef ISEARCH_FG_LITE_MAPWRAPPER_H
#define ISEARCH_FG_LITE_MAPWRAPPER_H

#include <unordered_map>
#include <vector>
#include "autil/ConstString.h"

template<typename T>
struct HasherWrapper {
    size_t operator()(T v) const {
        return std::hash<T>()(v);
    }
};

template<>
struct HasherWrapper<autil::ConstString> {
    // get first 2 and last 6 characters as hash value.
    // keep a balance between speed and uniqueness.
    size_t operator()(autil::ConstString str) const {
        size_t hashValue = 0;
        char *hashValueBuf = (char*)&hashValue;
        size_t copySize;
        if ( str.size() > 6 ) {
            hashValueBuf[6] = str.data()[0];
            hashValueBuf[7] = str.data()[1];
            copySize = 6;
        } else {
            copySize = str.size();
        }
        const char *strBuf = str.data() + str.size() - copySize;
        for (size_t i = 0; i < copySize; i++) {
            hashValueBuf[i] = strBuf[i];
        }
        return hashValue;
    }
};

template<>
struct HasherWrapper<autil::MultiValueType<char>> {
    size_t operator()(autil::MultiValueType<char> str) const {
        return HasherWrapper<autil::ConstString>()(autil::ConstString(str.data(), str.size()));
    }
};

template<typename K, typename V>
using map_alloc = autil::mem_pool::pool_allocator<std::pair<const K, V>>;

template<typename T>
using trivial_alloc = autil::mem_pool::pool_allocator<T>;

template<typename K, typename V>
using MapWrapper = std::unordered_map<K, V, HasherWrapper<K>, std::equal_to<K>, map_alloc<K, V>>;

template<typename K, typename V>
// using pair_alloc = autil::mem_pool::pool_allocator<std::pair<const K, std::pair<V, V>>>;
using pair_alloc = autil::mem_pool::pool_allocator<std::pair<K, std::pair<V, V>>>;

template<typename K, typename V>
using PairValueType = std::pair<K, std::pair<V, V>>;

template<typename K, typename V>
// using PairValueList = std::vector<PairValueType<K, V>, pair_alloc<K, std::pair<V, V>>>;
using PairValueList = std::vector<PairValueType<K, V>, trivial_alloc<PairValueType<K, V>>>;

#endif

