#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <type_traits>

#include "autil/CommonMacros.h"
#include "autil/LongHashValue.h"
#include "autil/MultiValueFormatter.h"
#include "autil/StringTokenizer.h"

namespace autil {
template <typename T> struct IsMultiType;
}  // namespace autil

namespace autil {

const char MULTI_VALUE_DELIMITER = '\x1D';

template <typename T>
class MultiValueType
{
public:
    typedef T type;
    typedef T value_type;
public:
    MultiValueType(const void* buffer = NULL)
        : _offsetInfo(INVALID_OFFSET_INFO)
    {
        init(buffer);
    }

    MultiValueType(const MultiValueType& other) {
        init(other.getBaseAddress());
    }

    MultiValueType<T>& operator = (const MultiValueType<T>& other) {
        init(other.getBaseAddress());
        return *this;
    }

    ~MultiValueType() = default;
public:
    void init(const void* buffer) {
        if (!buffer) {
            _offsetInfo = INVALID_OFFSET_INFO;
        } else {
            _offsetInfo = (char*)buffer - (char*)this;
        }
    }

    T operator[](uint32_t idx) const {
        return data()[idx];
    }

    const T& getRef(uint32_t idx) const {
        return data()[idx];
    }

    uint32_t size() const {
        if (unlikely(!isOffsetValid())) {
            return 0;
        }

        const char* data = getBaseAddress();
        size_t countLen = 0;
        uint32_t ret =  MultiValueFormatter::decodeCount(data, countLen);
        return (ret == MultiValueFormatter::VAR_NUM_NULL_FIELD_VALUE_COUNT) ? 0 : ret;
    }

    uint32_t length() const {
        if (unlikely(!isOffsetValid())) {
            return 0;
        }
        const char* data = getBaseAddress();
        return MultiValueFormatter::getEncodedCountFromFirstByte(*(uint8_t*)data)
            + size() * sizeof(T);
    }

    bool isNull() const {
        const char* data = getBaseAddress();
        if (data == NULL) {
            return false;
        }
        size_t countLen = 0;
        return MultiValueFormatter::decodeCount(data, countLen) ==
            MultiValueFormatter::VAR_NUM_NULL_FIELD_VALUE_COUNT;
    }

    const T* data() const {
        if (unlikely(!isOffsetValid())) {
            return NULL;
        }
        const char* data = getBaseAddress();
        size_t countLen = 0;
        if (MultiValueFormatter::decodeCount(data, countLen) ==
            MultiValueFormatter::VAR_NUM_NULL_FIELD_VALUE_COUNT)
        {
            return NULL;
        }
        return (const T*)(data + countLen);
    }

    const char* getBaseAddress() const {
        if (unlikely(!isOffsetValid())) {
            return NULL;
        }
        return (char*)this + _offsetInfo;
    }

public:
    bool operator == (const MultiValueType<T>& other) const {
        uint32_t recordNum = size();
        if (recordNum != other.size()) {
            return false;
        }
        if (isNull() != other.isNull()) {
            return false;
        }
        for (uint32_t i = 0; i < recordNum; ++i) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator != (const MultiValueType<T>& other) const {
        return !(*this == other);
    }

    bool operator > (const MultiValueType<T>& other) const {
        uint32_t recordNum = size();
        uint32_t otherRecordNum = other.size();
        uint32_t size = recordNum < otherRecordNum ? recordNum : otherRecordNum;
        for (uint32_t i = 0; i < size; ++i) {
            if ((*this)[i] > other[i]) {
                return true;
            } else if ((*this)[i] < other[i]) {
                return false;
            }
        }
        if (recordNum > otherRecordNum) {
            return true;
        }
        return false;
    }

    bool operator < (const MultiValueType<T>& other) const {
        uint32_t recordNum = size();
        uint32_t otherRecordNum = other.size();
        uint32_t size = recordNum < otherRecordNum ? recordNum : otherRecordNum;
        for (uint32_t i = 0; i < size; ++i) {
            if ((*this)[i] < other[i]) {
                return true;
            }
            else if ((*this)[i] > other[i]) {
                return false;
            }
        }
        if (recordNum < otherRecordNum) {
            return true;
        }
        return false;
    }

    bool operator >= (const MultiValueType<T>& other) const {
        return !(*this < other);
    }

    bool operator <= (const MultiValueType<T>& other) const {
        return !(*this > other);
    }

#define DEFINE_UNSUPPORTED_OPERATOR(OpType, ReturnType)                 \
    ReturnType operator OpType(const MultiValueType<T>& other) const    \
    {                                                                   \
        assert(false);                                                  \
        return ReturnType();                                            \
    }

    DEFINE_UNSUPPORTED_OPERATOR(&&, bool); // std::logical_and
    DEFINE_UNSUPPORTED_OPERATOR(||, bool); // std::logical_or
    DEFINE_UNSUPPORTED_OPERATOR(&, MultiValueType<T>); // bit_and
    DEFINE_UNSUPPORTED_OPERATOR(|, MultiValueType<T>); // bit_or
    DEFINE_UNSUPPORTED_OPERATOR(^, MultiValueType<T>); // bit_xor
    DEFINE_UNSUPPORTED_OPERATOR(+, MultiValueType<T>); // plus
    DEFINE_UNSUPPORTED_OPERATOR(-, MultiValueType<T>); // minus
    DEFINE_UNSUPPORTED_OPERATOR(*, MultiValueType<T>); // multiplies
    DEFINE_UNSUPPORTED_OPERATOR(/, MultiValueType<T>); // divide
#undef DEFINE_UNSUPPORTED_OPERATOR

    bool operator == (const std::string& fieldOfInput) const {
        autil::StringTokenizer st(fieldOfInput, std::string(1, MULTI_VALUE_DELIMITER),
                autil::StringTokenizer::TOKEN_TRIM |
                autil::StringTokenizer::TOKEN_IGNORE_EMPTY);

        if (size() == st.getNumTokens()) {
            for (size_t i = 0; i < st.getNumTokens(); ++i) {
                T valueOfInput;
                StrToT(st[i], valueOfInput);
                if (!CheckT(valueOfInput, (*this)[i])) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool operator != (const std::string& fieldOfInput) const {
        return !(*this == fieldOfInput);
    }

private:
    static const int64_t INVALID_OFFSET_INFO = 0;

private:
    bool isOffsetValid() const {
        return _offsetInfo != INVALID_OFFSET_INFO;
    }

private:
    int64_t _offsetInfo;
};

////////////////////////////////////////////////////////////////////////////////////////

template<>
inline bool MultiValueType<char>::operator == (const std::string& fieldOfInput) const {
    return std::string(data(), size()) == fieldOfInput;
}

template<>
inline bool MultiValueType<char>::operator != (const std::string& fieldOfInput) const {
    return std::string(data(), size()) != fieldOfInput;
}

template<>
inline MultiValueType<char> MultiValueType<MultiValueType<char> >::operator [] (uint32_t idx) const {
    size_t recordNum = size();
    const char* data = getBaseAddress();
    assert(data);
    size_t countLen = MultiValueFormatter::getEncodedCountFromFirstByte(*(const uint8_t*)data);
    const char* offsetLenAddr =  data + countLen;
    assert(idx < recordNum);
    uint8_t offsetItemLen = *(const uint8_t*)offsetLenAddr;
    const char* offsetAddr = (const char*)offsetLenAddr + sizeof(uint8_t);
    uint32_t offset = MultiValueFormatter::getOffset(offsetAddr, offsetItemLen, idx);
    const char* strDataAddr = offsetAddr + offsetItemLen * recordNum + offset;
    return MultiValueType<char>(strDataAddr);
}

template <>
inline const MultiValueType<char>* MultiValueType<MultiValueType<char> >::data() const {
    return NULL;
}

template<>
inline uint32_t MultiValueType<MultiValueType<char> >::length() const {
    if (unlikely(!isOffsetValid())) {
        return 0;
    }
    size_t countLen = 0;
    const char* data = getBaseAddress();
    uint32_t recordNum = MultiValueFormatter::decodeCount(data, countLen);
    if (recordNum == 0) {
        return countLen + sizeof(uint8_t);
    }
    if (recordNum == MultiValueFormatter::VAR_NUM_NULL_FIELD_VALUE_COUNT) {
        return MultiValueFormatter::getEncodedCountLength(
                MultiValueFormatter::VAR_NUM_NULL_FIELD_VALUE_COUNT);
    }
    const uint8_t* offsetItemLenAddr = (const uint8_t*)(data + countLen);
    uint8_t offsetItemLen = *offsetItemLenAddr;
    const char* offsetAddr = (const char*)offsetItemLenAddr + sizeof(uint8_t);
    uint32_t offset = MultiValueFormatter::getOffset(
            offsetAddr, offsetItemLen, recordNum - 1);
    const char* dataAddr = offsetAddr + offsetItemLen * recordNum + offset;

    size_t encodeCountLen = 0;
    uint32_t length = MultiValueFormatter::decodeCount(
            dataAddr, encodeCountLen);
    return dataAddr - (const char*)data + encodeCountLen + length;
}

#define MULTI_VALUE_TYPE_MACRO_HELPER(MY_MACRO)         \
    MY_MACRO(MultiValueType<bool>);                     \
    MY_MACRO(MultiValueType<int8_t>);                   \
    MY_MACRO(MultiValueType<uint8_t>);                  \
    MY_MACRO(MultiValueType<int16_t>);                  \
    MY_MACRO(MultiValueType<uint16_t>);                 \
    MY_MACRO(MultiValueType<int32_t>);                  \
    MY_MACRO(MultiValueType<uint32_t>);                 \
    MY_MACRO(MultiValueType<int64_t>);                  \
    MY_MACRO(MultiValueType<uint64_t>);                 \
    MY_MACRO(MultiValueType<autil::uint128_t>);          \
    MY_MACRO(MultiValueType<float>);                    \
    MY_MACRO(MultiValueType<double>);                   \
    MY_MACRO(MultiValueType<char>);                     \
    MY_MACRO(MultiValueType<MultiValueType<char> >);

// define identifiers
namespace {
enum {
    Bool,
    Int8,
    Uint8,
    Int16,
    Uint16,
    Int32,
    UInt32,
    Int64,
    Uint64,
    Uint128,
    Float,
    Double,
    Char,
    String,
};
}

#define MULTI_VALUE_TYPE_MACRO_HELPER_2(MY_MACRO)               \
    MY_MACRO(MultiValueType<bool>, Bool);                       \
    MY_MACRO(MultiValueType<int8_t>, Int8);                     \
    MY_MACRO(MultiValueType<uint8_t>, UInt8);                   \
    MY_MACRO(MultiValueType<int16_t>, Int16);                   \
    MY_MACRO(MultiValueType<uint16_t>, UInt16);                 \
    MY_MACRO(MultiValueType<int32_t>, Int32);                   \
    MY_MACRO(MultiValueType<uint32_t>, UInt32);                 \
    MY_MACRO(MultiValueType<int64_t>, Int64);                   \
    MY_MACRO(MultiValueType<uint64_t>, UInt64);                 \
    MY_MACRO(MultiValueType<autil::uint128_t>, UInt128);        \
    MY_MACRO(MultiValueType<float>, Float);                     \
    MY_MACRO(MultiValueType<double>, Double);                   \
    MY_MACRO(MultiValueType<char>, Char);                       \
    MY_MACRO(MultiValueType<MultiValueType<char> >, String);

#define MULTI_VALUE_TYPEDEF_1(type, suffix)             \
    typedef type Multi##suffix;
MULTI_VALUE_TYPE_MACRO_HELPER_2(MULTI_VALUE_TYPEDEF_1);
#undef MULTI_VALUE_TYPEDEF_1

#define MULTI_VALUE_TYPEDEF_2(type, suffix)             \
    typedef type FixedNum##suffix;
MULTI_VALUE_TYPE_MACRO_HELPER_2(MULTI_VALUE_TYPEDEF_2);
#undef MULTI_VALUE_TYPEDEF_2

#define MULTI_VALUE_OSTREAM(type)             \
    std::ostream& operator <<(std::ostream& stream, type value);
MULTI_VALUE_TYPE_MACRO_HELPER(MULTI_VALUE_OSTREAM);
#undef MULTI_VALUE_OSTREAM

template<typename T>
struct IsMultiType {
    typedef std::false_type type;
};

#define IS_MULTI_TYPE(m_type)                   \
    template<>                                  \
    struct IsMultiType<m_type> {                \
        typedef std::true_type type;            \
    };
MULTI_VALUE_TYPE_MACRO_HELPER(IS_MULTI_TYPE);
#undef IS_MULTI_TYPE

}
