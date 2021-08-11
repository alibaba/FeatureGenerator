#ifndef ISEARCH_FG_LITE_ANYCONVERT_H
#define ISEARCH_FG_LITE_ANYCONVERT_H

#include "autil/mem_pool/Pool.h"
#include "autil/StringUtil.h"
#include "fg_lite/feature/Feature.h"
#include "fg_lite/feature/FeatureFormatter.h"

namespace fg_lite {

template<typename T>
using IsStringType = std::integral_constant<bool,
                                            std::is_same<T, autil::MultiChar>::value ||
                                            std::is_same<T, autil::ConstString>::value ||
                                            std::is_same<T, std::string>::value>;


// only ConstString string ToType supported
template<typename FromType, typename ToType>
typename std::enable_if<!IsStringType<FromType>::value && !IsStringType<ToType>::value, ToType>::type
    anyconvert(const FromType &value, autil::mem_pool::Pool *pool) {
    return value;
}

template<typename FromType, typename ToType>
typename std::enable_if<IsStringType<FromType>::value && IsStringType<ToType>::value, ToType>::type
    anyconvert(const FromType &value, autil::mem_pool::Pool *pool) {
    return autil::ConstString(value.data(), value.size());
}

template<typename FromType, typename ToType>
typename std::enable_if<IsStringType<FromType>::value && !IsStringType<ToType>::value, ToType>::type
    anyconvert(const FromType &value, autil::mem_pool::Pool *pool) {
    return autil::StringUtil::fromString<ToType>(std::string(value.data(), value.size()));
}

template<typename FromType, typename ToType>
typename std::enable_if<!IsStringType<FromType>::value && IsStringType<ToType>::value, ToType>::type
    anyconvert(const FromType &value, autil::mem_pool::Pool *pool) {
    auto buffer = FeatureFormatter::FeatureBuffer(cp_alloc(pool));
    FeatureFormatter::fillFeatureToBuffer(value, buffer);
    return autil::ConstString(buffer.data(), buffer.size());
}

template<typename T>
struct Str2ConstStr {
    typedef T type;
    type operator()(const T &value) {
        return value;
    }
};

template<>
struct Str2ConstStr<std::string> {
    typedef autil::ConstString type;
    type operator()(const std::string &value) {
        return autil::ConstString(value.data(), value.size());
    }
};

template<>
struct Str2ConstStr<autil::MultiChar> {
    typedef autil::ConstString type;
    type operator()(const autil::MultiChar &value) {
        return autil::ConstString(value.data(), value.size());
    }
};


}

#endif //ISEARCH_FG_LITE_ANYCONVERT_H
