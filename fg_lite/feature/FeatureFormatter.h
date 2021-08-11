#ifndef ISEARCH_FG_LITE_FEATUREFORMATTER_H
#define ISEARCH_FG_LITE_FEATUREFORMATTER_H

#include "autil/Log.h"
#include "autil/MultiValueType.h"
#include "autil/ConstString.h"
#include "autil/mem_pool/pool_allocator.h"
#include <cmath>

namespace fg_lite {

static const char two_ASCII_digits[100][2] = {
    {'0','0'}, {'0','1'}, {'0','2'}, {'0','3'}, {'0','4'},
    {'0','5'}, {'0','6'}, {'0','7'}, {'0','8'}, {'0','9'},
    {'1','0'}, {'1','1'}, {'1','2'}, {'1','3'}, {'1','4'},
    {'1','5'}, {'1','6'}, {'1','7'}, {'1','8'}, {'1','9'},
    {'2','0'}, {'2','1'}, {'2','2'}, {'2','3'}, {'2','4'},
    {'2','5'}, {'2','6'}, {'2','7'}, {'2','8'}, {'2','9'},
    {'3','0'}, {'3','1'}, {'3','2'}, {'3','3'}, {'3','4'},
    {'3','5'}, {'3','6'}, {'3','7'}, {'3','8'}, {'3','9'},
    {'4','0'}, {'4','1'}, {'4','2'}, {'4','3'}, {'4','4'},
    {'4','5'}, {'4','6'}, {'4','7'}, {'4','8'}, {'4','9'},
    {'5','0'}, {'5','1'}, {'5','2'}, {'5','3'}, {'5','4'},
    {'5','5'}, {'5','6'}, {'5','7'}, {'5','8'}, {'5','9'},
    {'6','0'}, {'6','1'}, {'6','2'}, {'6','3'}, {'6','4'},
    {'6','5'}, {'6','6'}, {'6','7'}, {'6','8'}, {'6','9'},
    {'7','0'}, {'7','1'}, {'7','2'}, {'7','3'}, {'7','4'},
    {'7','5'}, {'7','6'}, {'7','7'}, {'7','8'}, {'7','9'},
    {'8','0'}, {'8','1'}, {'8','2'}, {'8','3'}, {'8','4'},
    {'8','5'}, {'8','6'}, {'8','7'}, {'8','8'}, {'8','9'},
    {'9','0'}, {'9','1'}, {'9','2'}, {'9','3'}, {'9','4'},
    {'9','5'}, {'9','6'}, {'9','7'}, {'9','8'}, {'9','9'}
};

class FeatureFormatter
{
public:
    typedef std::vector<char, autil::mem_pool::pool_allocator<char>> FeatureBuffer;
public:
    FeatureFormatter();
    ~FeatureFormatter() = default;
private:
    FeatureFormatter(const FeatureFormatter &);
    FeatureFormatter& operator=(const FeatureFormatter &);
public:
    template <typename T>
    static void fillFeatureToBuffer(const T &value, FeatureBuffer &buffer);

    static void FastUInt32ToBufferLeft(uint32_t u, FeatureBuffer &buffer);
    static void FastInt32ToBufferLeft(int32_t i, FeatureBuffer &buffer);

    static void FastUInt64ToBufferLeft(uint64_t u64, FeatureBuffer &buffer);
    static void FastInt64ToBufferLeft(int64_t i, FeatureBuffer &buffer);

public:
    template <typename T>
    static bool isInvalidValue(const T &value);

    template <typename T>
    static bool getInvalidValue(T &value);

private:
    AUTIL_LOG_DECLARE();
};
////////////////////////////////////////////////////

template <typename T>
inline bool FeatureFormatter::getInvalidValue(T &value) {
    AUTIL_LOG(ERROR, "unsupported get InValidValuetype [%s]", typeid(T).name());
    assert(false);
    return false;
}

template <>
inline bool FeatureFormatter::getInvalidValue(int64_t &value) {
    value = std::numeric_limits<int64_t>::max();
    return true;
}

#if defined(__OSX__)
template <>
inline bool FeatureFormatter::getInvalidValue(long long int &value) {
    value = std::numeric_limits<long long int>::max();
    return true;
}
#endif

template <>
inline bool FeatureFormatter::getInvalidValue(double &value) {
    value = std::numeric_limits<double>::quiet_NaN();
    return true;
}

template <>
inline bool FeatureFormatter::getInvalidValue(std::string &value) {
    value = "";
    return true;
}

template <typename T>
inline void FeatureFormatter::fillFeatureToBuffer(const T &value, FeatureBuffer &buffer) {
    AUTIL_LOG(ERROR, "unsupported type [%s]", typeid(T).name());
    assert(false); // do not support
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const bool &value, FeatureBuffer &buffer)
{
    if (value) {
        FastInt32ToBufferLeft(1, buffer);
    } else {
        FastInt32ToBufferLeft(0, buffer);
    }
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const char &value, FeatureBuffer &buffer)
{
    buffer.push_back(value);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const int8_t &value, FeatureBuffer &buffer)
{
    FastInt32ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const uint8_t &value, FeatureBuffer &buffer)
{
    FastInt32ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const int16_t &value, FeatureBuffer &buffer)
{
    FastInt32ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const uint16_t &value, FeatureBuffer &buffer)
{
    FastInt32ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const int32_t &value, FeatureBuffer &buffer)
{
    FastInt32ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const uint32_t &value, FeatureBuffer &buffer)
{
    FastUInt32ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const int64_t &value, FeatureBuffer &buffer)
{
    FastInt64ToBufferLeft(value, buffer);
}

#if defined(__OSX__)
template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const long long int &value, FeatureBuffer &buffer)
{
    FastInt64ToBufferLeft((int64_t)value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const long long unsigned &value, FeatureBuffer &buffer)
{
    FastInt64ToBufferLeft((uint64_t)value, buffer);
}
#endif

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const uint64_t &value, FeatureBuffer &buffer)
{
    FastUInt64ToBufferLeft(value, buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const double &value, FeatureBuffer &buffer)
{
    char formatBuffer[1024];
    size_t ret = snprintf(formatBuffer, 1024, "%.0f", value);
    buffer.insert(buffer.end(), formatBuffer, formatBuffer+ret);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const float &value, FeatureBuffer &buffer)
{
    fillFeatureToBuffer(double(value), buffer);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
    const autil::MultiChar &value, FeatureBuffer &buffer)
{
    char* data = (char*)value.data();
    size_t dataLen = value.size();
    buffer.insert(buffer.end(), data, data + dataLen);
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const std::string &value, FeatureBuffer &buffer)
{
    buffer.insert(buffer.end(), value.begin(), value.end());
}

template <>
inline void FeatureFormatter::fillFeatureToBuffer(
        const autil::ConstString &value, FeatureBuffer &buffer)
{
    buffer.insert(buffer.end(), value.begin(), value.end());
}

inline void FeatureFormatter::FastUInt32ToBufferLeft(uint32_t u, FeatureBuffer &buffer) {
    int digits;
    const char *ASCII_digits = NULL;
    // The idea of this implementation is to trim the number of divides to as few
    // as possible by using multiplication and subtraction rather than mod (%),
    // and by outputting two digits at a time rather than one.
    // The huge-number case is first, in the hopes that the compiler will output
    // that case in one branch-free block of code, and only output conditional
    // branches into it from below.
    if (u >= 1000000000) {  // >= 1,000,000,000
        digits = u / 100000000;  // 100,000,000
        ASCII_digits = two_ASCII_digits[digits];
        buffer.push_back(ASCII_digits[0]);
        buffer.push_back(ASCII_digits[1]);
    sublt100_000_000:
        u -= digits * 100000000;  // 100,000,000
    lt100_000_000:
        digits = u / 1000000;  // 1,000,000
        ASCII_digits = two_ASCII_digits[digits];
        buffer.push_back(ASCII_digits[0]);
        buffer.push_back(ASCII_digits[1]);
    sublt1_000_000:
        u -= digits * 1000000;  // 1,000,000
    lt1_000_000:
        digits = u / 10000;  // 10,000
        ASCII_digits = two_ASCII_digits[digits];
        buffer.push_back(ASCII_digits[0]);
        buffer.push_back(ASCII_digits[1]);
    sublt10_000:
        u -= digits * 10000;  // 10,000
    lt10_000:
        digits = u / 100;
        ASCII_digits = two_ASCII_digits[digits];
        buffer.push_back(ASCII_digits[0]);
        buffer.push_back(ASCII_digits[1]);
    sublt100:
        u -= digits * 100;
    lt100:
        digits = u;
        ASCII_digits = two_ASCII_digits[digits];
        buffer.push_back(ASCII_digits[0]);
        buffer.push_back(ASCII_digits[1]);
    done:
        return;
    }

    if (u < 100) {
        digits = u;
        if (u >= 10) goto lt100;
        buffer.push_back('0' + digits);
        goto done;
    }
    if (u  <  10000) {   // 10,000
        if (u >= 1000) goto lt10_000;
        digits = u / 100;
        buffer.push_back('0' + digits);
        goto sublt100;
    }
    if (u  <  1000000) {   // 1,000,000
        if (u >= 100000) goto lt1_000_000;
        digits = u / 10000;  //    10,000
        buffer.push_back('0' + digits);
        goto sublt10_000;
    }
    if (u  <  100000000) {   // 100,000,000
        if (u >= 10000000) goto lt100_000_000;
        digits = u / 1000000;  //   1,000,000
        buffer.push_back('0' + digits);
        goto sublt1_000_000;
    }
    // we already know that u < 1,000,000,000
    digits = u / 100000000;   // 100,000,000
    buffer.push_back('0' + digits);
    goto sublt100_000_000;
}

inline void FeatureFormatter::FastInt32ToBufferLeft(int32_t i, FeatureBuffer &buffer) {
    uint32_t u;
    if (i >= 0) {
        u = i;
    } else {
        buffer.push_back('-');
        u = -i;
    }
    FastUInt32ToBufferLeft(u, buffer);
}

inline void FeatureFormatter::FastUInt64ToBufferLeft(uint64_t u64, FeatureBuffer &buffer) {
    int digits;
    const char *ASCII_digits = NULL;

    uint32_t u = static_cast<uint32_t>(u64);
    if (u == u64) return FastUInt32ToBufferLeft(u, buffer);

    uint64_t top_11_digits = u64 / 1000000000;
    FastUInt64ToBufferLeft(top_11_digits, buffer);
    u = u64 - (top_11_digits * 1000000000);

    digits = u / 10000000;  // 10,000,000
    // GOOGLE_DCHECK_LT(digits, 100);
    ASCII_digits = two_ASCII_digits[digits];
    buffer.push_back(ASCII_digits[0]);
    buffer.push_back(ASCII_digits[1]);
    u -= digits * 10000000;  // 10,000,000
    digits = u / 100000;  // 100,000
    ASCII_digits = two_ASCII_digits[digits];
    buffer.push_back(ASCII_digits[0]);
    buffer.push_back(ASCII_digits[1]);
    u -= digits * 100000;  // 100,000
    digits = u / 1000;  // 1,000
    ASCII_digits = two_ASCII_digits[digits];
    buffer.push_back(ASCII_digits[0]);
    buffer.push_back(ASCII_digits[1]);
    u -= digits * 1000;  // 1,000
    digits = u / 10;
    ASCII_digits = two_ASCII_digits[digits];
    buffer.push_back(ASCII_digits[0]);
    buffer.push_back(ASCII_digits[1]);
    u -= digits * 10;
    digits = u;
    buffer.push_back('0' + digits);
}

inline void FeatureFormatter::FastInt64ToBufferLeft(int64_t i, FeatureBuffer &buffer) {
    uint64_t u;
    if (i >= 0) {
        u = i;
    } else {
        buffer.push_back('-');
        u = -i;
    }
    return FastUInt64ToBufferLeft(u, buffer);
}

template <typename T>
inline bool FeatureFormatter::isInvalidValue(const T &value) {
    return std::numeric_limits<T>::max() == value;
}

template <>
inline bool FeatureFormatter::isInvalidValue(const bool &value) {
    return false;
}

template <>
inline bool FeatureFormatter::isInvalidValue(const autil::MultiChar &value) {
    return false;
}

template <>
inline bool FeatureFormatter::isInvalidValue(const double &value) {
    return std::isnan(value);
}

template <>
inline bool FeatureFormatter::isInvalidValue(const std::string &value) {
    return false;
}

}

#endif //ISEARCH_FG_LITE_FEATUREFORMATTER_H
