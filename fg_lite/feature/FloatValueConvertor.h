#ifndef ISEARCH_FG_LITE_FLOATVALUECONVERTOR_H
#define ISEARCH_FG_LITE_FLOATVALUECONVERTOR_H

#include <stdexcept>
#include "autil/Log.h"
#include "autil/MultiValueType.h"
#include "autil/StringUtil.h"

namespace fg_lite {

class FloatValueConvertor {
public:
    FloatValueConvertor();
    ~FloatValueConvertor() = default;
private:
    FloatValueConvertor(const FloatValueConvertor &);
    FloatValueConvertor& operator=(const FloatValueConvertor &);
public:
    template <typename T>
    static bool convertToFloat(const T &value, float &result);

    template <typename T>
    static bool convertToDouble(const T &value, double &result);
private:
    AUTIL_LOG_DECLARE();
};

template <typename T>
inline bool FloatValueConvertor::convertToFloat(const T &value, float &result) {
    AUTIL_LOG(ERROR, "unsupported type [%s]", typeid(T).name());
    assert(false); // do not support
    return false;
}
template <typename T>
inline bool FloatValueConvertor::convertToDouble(const T &value, double &result) {
    AUTIL_LOG(ERROR, "unsupported type [%s]", typeid(T).name());
    assert(false); // do not support
    return false;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const bool &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const int8_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const uint8_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const int16_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const uint16_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const int32_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const uint32_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const int64_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const uint64_t &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const float &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const double &value, float &result) {
    result = (float) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToFloat(const autil::MultiChar &value, float &result) {
    uint32_t size = value.size();
    if (unlikely(size == 0)) {
        return false;
    }
    if (unlikely(size > 9)) {
        return autil::StringUtil::fromString<float>(std::string(value.data(), value.size()), result);
    }
    char copyed[10] = {0};
    std::copy_n(value.data(), size, copyed);
    return autil::StringUtil::strToFloat(copyed, result);
}

template <>
inline bool FloatValueConvertor::convertToFloat(const std::string &value, float &result)
{
    return autil::StringUtil::fromString<float>(value, result);
}

//
// =========================== Double ========================= //
//
template <>
inline bool FloatValueConvertor::convertToDouble(const bool &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const int8_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const uint8_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const int16_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const uint16_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const int32_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const uint32_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const int64_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const uint64_t &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const float &value, double &result) {
    result = (double) value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const double &value, double &result) {
    result = value;
    return true;
}

template <>
inline bool FloatValueConvertor::convertToDouble(const autil::MultiChar &value, double &result) {
    if (value.size() <= 0) {
        result = 0.0;
        return false;
    }
    return autil::StringUtil::fromString<double>(std::string(value.data(), value.size()), result);
}

template <>
inline bool FloatValueConvertor::convertToDouble(const std::string &value, double &result)
{
    if (value.size() == 0) {
        result = 0.0;
        return false;
    }
    return autil::StringUtil::fromString<double>(value, result);
}

}

#endif //ISEARCH_FG_LITE_FLOATVALUECONVERTOR_H
