#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <string>

#include "autil/LongHashValue.h"
#include "autil/MultiValueFormatter.h"
#include "autil/MultiValueType.h"
#include "autil/StringTokenizer.h"

namespace autil {

template <typename T>
class CountedMultiValueType
{
public:
    typedef T type;
    
public:
    CountedMultiValueType(const MultiValueType<T>& mt)
        : _offsetInfo(INVALID_OFFSET_INFO)
        , _fixCount(0)
        , _isNull(mt.isNull())
    {    
        uint32_t count = mt.size();
        const char* buffer = mt.getBaseAddress();
        if (buffer) {
            buffer = buffer + MultiValueFormatter::getEncodedCountLength(count);
        }
        init(buffer, count, _isNull);
    }
    
    CountedMultiValueType(const void* buffer = NULL, uint32_t count = 0)
        : _offsetInfo(INVALID_OFFSET_INFO)
        , _fixCount(0)
        , _isNull(false)
    {
        init(buffer, count);
    }
    
    CountedMultiValueType(const CountedMultiValueType& other) {
        init(other.getBaseAddress(), other.size());
    }

    CountedMultiValueType<T>& operator = (const CountedMultiValueType<T>& other) {
        init(other.getBaseAddress(), other.size(), other.isNull());
        return *this;
    }

    ~CountedMultiValueType() {}

public:
    void init(const void* buffer, uint32_t count, bool isNullValue = false) {
        if (!buffer) {
            _offsetInfo = INVALID_OFFSET_INFO;
            _fixCount = 0;
        } else {
            _offsetInfo = (char*)buffer - (char*)this;
            _fixCount = count;
        }
        _isNull = isNullValue;
    }

    bool isNull() const {
        return _isNull;
    }
    
    T operator[](uint32_t idx) const {
        return data()[idx];
    }

    const T& getRef(uint32_t idx) const {
        return data()[idx];
    }

    uint32_t size() const {
        return _fixCount;
    }

    uint32_t length() const {
        return _fixCount * sizeof(T);
    }

    const T* data() const {
        if (unlikely(_fixCount == 0)) {
            return NULL;
        }
        return (const T*)(getBaseAddress());
    }

    const char* getBaseAddress() const {
        if (likely(_fixCount > 0)) {
            return (char*)this + _offsetInfo;
        }
        return NULL;
    }

public:
    bool operator == (const CountedMultiValueType<T>& other) const {
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

    bool operator != (const CountedMultiValueType<T>& other) const {
        return !(*this == other);
    }

    bool operator > (const CountedMultiValueType<T>& other) const {
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

    bool operator < (const CountedMultiValueType<T>& other) const {
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

    bool operator >= (const CountedMultiValueType<T>& other) const {
        return !(*this < other);
    }

    bool operator <= (const CountedMultiValueType<T>& other) const {
        return !(*this > other);
    }

#define DEFINE_UNSUPPORTED_OPERATOR(OpType, ReturnType)                 \
    ReturnType operator OpType(const CountedMultiValueType<T>& other) const \
    {                                                                   \
        assert(false);                                                  \
        return ReturnType();                                            \
    }

    DEFINE_UNSUPPORTED_OPERATOR(&&, bool); // std::logical_and
    DEFINE_UNSUPPORTED_OPERATOR(||, bool); // std::logical_or
    DEFINE_UNSUPPORTED_OPERATOR(&, CountedMultiValueType<T>); // bit_and
    DEFINE_UNSUPPORTED_OPERATOR(|, CountedMultiValueType<T>); // bit_or
    DEFINE_UNSUPPORTED_OPERATOR(^, CountedMultiValueType<T>); // bit_xor
    DEFINE_UNSUPPORTED_OPERATOR(+, CountedMultiValueType<T>); // plus
    DEFINE_UNSUPPORTED_OPERATOR(-, CountedMultiValueType<T>); // minus
    DEFINE_UNSUPPORTED_OPERATOR(*, CountedMultiValueType<T>); // multiplies
    DEFINE_UNSUPPORTED_OPERATOR(/, CountedMultiValueType<T>); // divide
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
    int64_t _offsetInfo;
    uint32_t _fixCount;
    bool _isNull;
};

////////////////////////////////////////////////////////////////////////////////////////

template<> 
inline bool CountedMultiValueType<char>::operator == (const std::string& fieldOfInput) const {
    return std::string(data(), size()) == fieldOfInput;
}

template<>
inline MultiChar CountedMultiValueType<MultiChar>::operator [] (uint32_t idx) const {
    size_t recordNum = size();
    const char* offsetLenAddr = getBaseAddress();
    assert(idx < recordNum);
    uint8_t offsetItemLen = *(const uint8_t*)offsetLenAddr;
    const char* offsetAddr = (const char*)offsetLenAddr + sizeof(uint8_t);
    uint32_t offset = MultiValueFormatter::getOffset(offsetAddr, offsetItemLen, idx);
    const char* strDataAddr = offsetAddr + offsetItemLen * recordNum + offset;
    return MultiChar(strDataAddr);          
}

template <>
inline const MultiChar* CountedMultiValueType<MultiChar>::data() const {
    return NULL;
}

template<>
inline uint32_t CountedMultiValueType<MultiChar>::length() const {
    size_t recordNum = size();
    if (unlikely(recordNum == 0)) {
        return 0;
    }
    if (recordNum == 0) {
        return sizeof(uint8_t);
    }
    const uint8_t* offsetItemLenAddr = (const uint8_t*)getBaseAddress();
    uint8_t offsetItemLen = *offsetItemLenAddr;
    const char* offsetAddr = (const char*)offsetItemLenAddr + sizeof(uint8_t);
    uint32_t offset = MultiValueFormatter::getOffset(
            offsetAddr, offsetItemLen, recordNum - 1);
    const char* dataAddr = offsetAddr + offsetItemLen * recordNum + offset;

    size_t encodeCountLen = 0;
    uint32_t length = MultiValueFormatter::decodeCount(
            dataAddr, encodeCountLen);
    return dataAddr - (const char*)offsetItemLenAddr + encodeCountLen + length;
}

typedef CountedMultiValueType<int8_t> CountedMultiInt8;
typedef CountedMultiValueType<uint8_t> CountedMultiUInt8;
typedef CountedMultiValueType<int16_t> CountedMultiInt16;
typedef CountedMultiValueType<uint16_t> CountedMultiUInt16;
typedef CountedMultiValueType<int32_t> CountedMultiInt32;
typedef CountedMultiValueType<uint32_t> CountedMultiUInt32;
typedef CountedMultiValueType<int64_t> CountedMultiInt64;
typedef CountedMultiValueType<uint64_t> CountedMultiUInt64;
typedef CountedMultiValueType<float> CountedMultiFloat;
typedef CountedMultiValueType<double> CountedMultiDouble;
typedef CountedMultiValueType<char> CountedMultiChar;
typedef CountedMultiValueType<MultiChar> CountedMultiString;

///////////////////////// ostream
std::ostream& operator <<(std::ostream& stream, CountedMultiInt8 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiUInt8 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiInt16 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiUInt16 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiInt32 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiUInt32 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiInt64 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiUInt64 value);
std::ostream& operator <<(std::ostream& stream, CountedMultiFloat value);
std::ostream& operator <<(std::ostream& stream, CountedMultiDouble value);
std::ostream& operator <<(std::ostream& stream, CountedMultiChar value);
std::ostream& operator <<(std::ostream& stream, CountedMultiString value);
std::ostream& operator <<(std::ostream& stream, CountedMultiValueType<autil::uint128_t> value);

}
