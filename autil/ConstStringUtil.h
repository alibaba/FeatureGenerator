#pragma once

#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "autil/ConstString.h"
#include "autil/MultiValueCreator.h"
#include "autil/MultiValueType.h"
#include "autil/StringTokenizer.h"
#include "autil/StringUtil.h"
#include "autil/mem_pool/Pool.h"

namespace autil {

class ConstStringUtil
{
public:
    template<typename T>
    static bool fromString(const ConstString *value, T &val, mem_pool::Pool *pool);

    template <typename T>
    static bool fromString(const ConstString *value,
                           MultiValueType<T> &val,
                           mem_pool::Pool *pool);

    template <typename T>
    static T fromString(const ConstString &str);

    template <typename T>
    static bool fromString(const ConstString &str, T &val);

    static std::vector<ConstString> split(const ConstString& text, const std::string &sepStr, bool ignoreEmpty = true);
    static void split(std::vector<ConstString> &vec, const ConstString& text, const std::string &sepStr, bool ignoreEmpty = true);
    static void split(std::vector<ConstString> &vec, const ConstString& text, const char &sepChar, bool ignoreEmpty = true);
};

template <typename T>
inline T ConstStringUtil::fromString(const ConstString &str)
{
    T value;
    fromString(str, value);
    return value;
}

template <typename T>
inline bool ConstStringUtil::fromString(const ConstString &str, T &value)
{
    return StringUtil::fromString(str.toString(), value);
}

template <>
inline bool ConstStringUtil::fromString(const ConstString &str, std::string &value)
{
    value = str.toString();
    return true;
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, int8_t &value) {
    return StringUtil::strToInt8(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, uint8_t &value) {
    return StringUtil::strToUInt8(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, int16_t &value) {
    return StringUtil::strToInt16(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, uint16_t &value) {
    return StringUtil::strToUInt16(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, int32_t &value) {
    return StringUtil::strToInt32(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, uint32_t &value) {
    return StringUtil::strToUInt32(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, int64_t &value) {
    return StringUtil::strToInt64(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, uint64_t &value) {
    return StringUtil::strToUInt64(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, float &value) {
    return StringUtil::strToFloat(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, double &value) {
    return StringUtil::strToDouble(str.c_str(), value);
}

template<>
inline bool ConstStringUtil::fromString(const ConstString &str, bool &value) {
    return StringUtil::parseTrueFalse(str.c_str(), value);
}

template <typename T>
inline bool ConstStringUtil::fromString(const ConstString *value,
                                        T &val,
                                        mem_pool::Pool *pool)
{
    assert(value);
    return StringUtil::fromString<T>(value->toString(), val);
}

template <typename T>
inline bool ConstStringUtil::fromString(const ConstString *value,
                                        MultiValueType<T> &val,
                                        mem_pool::Pool *pool)
{
    assert(value);
    std::vector<T> valVec;
    StringUtil::fromString(
            value->toString(), valVec, std::string(1, MULTI_VALUE_DELIMITER));
    char *buffer = MultiValueCreator::createMultiValueBuffer<T>(valVec, pool);
    if (buffer == nullptr) {
        return false;
    }
    val.init(buffer);
    return true;
}

template <>
inline bool ConstStringUtil::fromString<char>(
        const ConstString *value,
        MultiChar &val,
        mem_pool::Pool *pool)
{
    assert(value);
    char *buffer = MultiValueCreator::createMultiValueBuffer(
            value->data(), value->size(), pool);
    if (buffer == nullptr) {
        return false;
    }
    val.init(buffer);
    return true;
}

template <>
inline bool ConstStringUtil::fromString<MultiChar>(
        const ConstString *value,
        MultiString &val,
        mem_pool::Pool *pool)
{
    assert(value);
    char *buffer = nullptr;
    if (value->empty()) {
        std::vector<std::string> vec;
        buffer = autil::MultiValueCreator::createMultiValueBuffer(vec, pool);
        val.init(buffer);
    } else {
        std::vector<autil::ConstString> vec = autil::StringTokenizer::constTokenize(
                *value, autil::MULTI_VALUE_DELIMITER, autil::StringTokenizer::TOKEN_LEAVE_AS);
        buffer = autil::MultiValueCreator::createMultiValueBuffer(vec, pool);
    }
    if (buffer == nullptr) {
        return false;
    }
    val.init(buffer);
    return true;
}

}
