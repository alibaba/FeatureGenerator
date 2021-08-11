#pragma once

#include <cxxabi.h>
#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "autil/ShortString.h"

namespace autil {

class StringUtil
{
public:
    static const std::string NULL_STRING;
    typedef bool (*CMP_CHAR_FUNC)(const char a, const char b);
public:
    StringUtil();
    ~StringUtil();
public:
    static void trim(std::string& str);
    static bool startsWith(const std::string &str, const std::string &prefix);
    static bool endsWith(const std::string &str, const std::string &suffix);
    static std::vector<std::string> split(const std::string& text, const std::string &sepStr, bool ignoreEmpty = true);
    static void split(std::vector<std::string> &vec, const std::string& text,
                      const char &sepChar, bool ignoreEmpty = true);
    static void split(std::vector<std::string> &vec, const std::string& text,
                      const std::string &sepStr, bool ignoreEmpty = true);
    static bool isSpace(const std::string& text);
    static bool isSpace(const ShortString& text);

    static void toUpperCase(char *str);
    static void toUpperCase(const char *str, std::string &retStr);
    static void toUpperCase(std::string &str);
    static void toLowerCase(char *str);
    static void toLowerCase(std::string &str);
    static void toLowerCase(const char *str, std::string &retStr);

    static bool strToInt8(const char* str, int8_t& value);
    static bool strToUInt8(const char* str, uint8_t& value);
    static bool strToInt16(const char* str, int16_t& value);
    static bool strToUInt16(const char* str, uint16_t& value);
    static bool strToInt32(const char* str, int32_t& value);
    static bool strToUInt32(const char* str, uint32_t& value);
    static bool strToInt64(const char* str, int64_t& value);
    static bool strToUInt64(const char* str, uint64_t& value);
    static bool strToFloat(const char *str, float &value);
    static bool strToDouble(const char *str, double &value);
    static bool hexStrToUint64(const char* str, uint64_t& value);
    static bool hexStrToInt64(const char* str, int64_t& value);
    static void uint64ToHexStr(uint64_t value, char* hexStr, int len);

    static std::string strToHexStr(const std::string &str);
    static std::string hexStrToStr(const std::string &hexStr);

    static uint32_t deserializeUInt32(const std::string& str);
    static void serializeUInt32(uint32_t value, std::string& str);

    static uint64_t deserializeUInt64(const std::string& str);
    static void serializeUInt64(uint64_t value, std::string& str);

    static int8_t strToInt8WithDefault(const char* str, int8_t defaultValue);
    static uint8_t strToUInt8WithDefault(const char* str, uint8_t defaultValue);
    static int16_t strToInt16WithDefault(const char* str, int16_t defaultValue);
    static uint16_t strToUInt16WithDefault(const char* str, uint16_t defaultValue);
    static int32_t strToInt32WithDefault(const char* str, int32_t defaultValue);
    static uint32_t strToUInt32WithDefault(const char* str, uint32_t defaultValue);
    static int64_t strToInt64WithDefault(const char* str, int64_t defaultValue);
    static uint64_t strToUInt64WithDefault(const char* str, uint64_t defaultValue);
    static float strToFloatWithDefault(const char* str, float defaultValue);
    static double strToDoubleWithDefault(const char* str, double defaultValue);

    static void replace(std::string &str, char oldValue, char newValue);
    static void replaceLast(std::string &str, const std::string &oldStr,
                            const std::string &newStr);
    static void replaceAll(std::string &str, const std::string &oldStr,
                           const std::string &newStr);

    static void sundaySearch(const std::string &text,
                             const std::string &key,
                             std::vector<size_t> &posVec,
                             bool caseSensitive = true);
    static void sundaySearch(const char *text, const char *key,
                             std::vector<size_t> &posVec);
    static void sundaySearch(const char *text, size_t textSize,
                             const char *key, size_t keySize,
                             std::vector<size_t> &posVec);

    static const std::string& getValueFromMap(const std::string& key,
            const std::map<std::string, std::string> &map);

    template<typename T>
    static std::string toString(const T &x);

    template<typename T>
    static std::string fToString(const T &x, const char *format = "%g");

    template<typename T>
    static T numberFromString(const std::string &str);

    template<typename T>
    static bool numberFromString(const std::string &str, T &val);

    template<typename T>
    static T fromString(const std::string &str);

    template<typename T>
    static bool fromString(const std::string &str, T &value);

    template<typename T>
    static void fromString(const std::vector<std::string> &strVec, std::vector<T> &vec);

    template<typename T>
    static void fromString(const std::string &str, std::vector<T> &vec, const std::string &delim);

    template<typename T>
    static void fromString(const std::string &str, std::vector<std::vector<T> > &vec, const std::string &delim, const std::string &delim2);

    template<typename T>
    static std::string toString(const std::vector<T> &x, const std::string &delim = " ");

    template<typename T>
    static std::string toString(const std::vector<std::string> &x, const std::string &delim = " ");

    template <typename Iterator>
    static std::string toString(Iterator first, Iterator last, const std::string &delim = " ");

    template<typename T>
    static std::string toString(const std::vector<std::vector<T> > &x, const std::string &delim1, const std::string &delim2);

    static std::string toString(const double &x, int32_t precision);

    static void getKVValue(const std::string& text, std::string& key, std::string& value,
                           const std::string& sep, bool isTrim = false);
    static bool parseTrueFalse(const std::string &str, bool &ret);

    template <typename T>
    static std::string toBinaryString(T x);

    template <typename T>
    static bool fromBinaryString(const std::string &str, T &x);

    template<typename T>
    static std::string getTypeString(const T &t);

    static bool tryConvertToDateInMonth(int64_t inputTs, std::string& str);

    static std::string formatString(const char* format, ...);

private:
    static std::stringstream* getStringStream();
    static void putStringStream(std::stringstream* ss);
    friend class StringStreamWrapper;
    class StringStreamWrapper {
    public:
        StringStreamWrapper(const std::string &str = "")
            : _ss(StringUtil::getStringStream()) {_ss->str(str);}
        ~StringStreamWrapper() {StringUtil::putStringStream(_ss);}
        template<typename T>
        StringStreamWrapper& operator << (const T &x) {
            *_ss << x;
            return *this;
        }
        template<typename T>
        StringStreamWrapper& operator >> (T &x) {
            *_ss >> x;
            return *this;
        }
        std::string str() {return _ss->str();}
        bool eof() {return _ss->eof();}
    private:
        std::stringstream *_ss;
    };
};

template<typename T>
inline T StringUtil::fromString(const std::string& str) {
    T value = T();
    fromString(str, value);
    return value;
}

template<typename T>
inline bool StringUtil::fromString(const std::string &str, T &value) {
    StringStreamWrapper iss(str);
    iss >> value;
    return iss.eof();
}

template<>
inline bool StringUtil::fromString(const std::string& str, std::string &value) {
    value = str;
    return true;
}

template<>
inline bool StringUtil::fromString(const std::string& str, int8_t &value) {
    bool ret = strToInt8(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, uint8_t &value) {
    bool ret = strToUInt8(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, int16_t &value) {
    bool ret = strToInt16(str.data(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, uint16_t &value) {
    bool ret = strToUInt16(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, int32_t &value) {
    bool ret = strToInt32(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, uint32_t &value) {
    bool ret = strToUInt32(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, int64_t &value) {
    bool ret = strToInt64(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, uint64_t &value) {
    bool ret = strToUInt64(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, float &value) {
    bool ret = strToFloat(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string& str, double &value) {
    bool ret = strToDouble(str.c_str(), value);
    return ret;
}

template<>
inline bool StringUtil::fromString(const std::string &str, bool &value) {
    return parseTrueFalse(str, value);
}

template<typename T>
inline T StringUtil::numberFromString(const std::string &str) {
    T value = T();
    numberFromString(str, value);
    return value;
}

template<typename T>
inline bool StringUtil::numberFromString(const std::string &str, T &val) {
    if (str.size() > 2) {
        size_t pos = 0;
        if (str[0] == '-') {
            pos = 1;
        }
        char ch1 = str[pos];
        char ch2 = str[pos + 1];
        if (ch1 == '0' && (ch2 == 'x' || ch2 == 'X')) {
            int64_t value;
            if (!hexStrToInt64(str.c_str(), value)) {
                return false;
            }
            if (value > std::numeric_limits<T>::max()
                || value < std::numeric_limits<T>::min())
            {
                return false;
            }
            val = (T)value;
            return true;
        }
    }
    return fromString<T>(str, val);
}

template<>
inline bool StringUtil::numberFromString(const std::string &str, uint64_t &val) {
    if (str.size() > 2) {
        size_t pos = 0;
        if (str[0] == '-') {
            pos = 1;
        }
        char ch1 = str[pos];
        char ch2 = str[pos + 1];
        if (ch1 == '0' && (ch2 == 'x' || ch2 == 'X')) {
            return hexStrToUint64(str.c_str(), val);
        }
    }
    return fromString<uint64_t>(str, val);
}

template<typename T>
inline std::string StringUtil::toString(const T &x) {
    StringStreamWrapper oss;
    oss << x;
    return oss.str();
}

template<>
inline std::string StringUtil::toString<int8_t>(const int8_t &x) {
    char buf[8] = {0,};
    snprintf(buf, sizeof(buf), "%d", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<uint8_t>(const uint8_t &x) {
    char buf[8] = {0,};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation="
    snprintf(buf, sizeof(buf), "%u", x);
#pragma GCC diagnostic pop
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<int16_t>(const int16_t &x) {
    char buf[16] = {0,};
    snprintf(buf, sizeof(buf), "%d", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<uint16_t>(const uint16_t &x) {
    char buf[16] = {0,};
    snprintf(buf, sizeof(buf), "%u", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<int32_t>(const int32_t &x) {
    char buf[32] = {0,};
    snprintf(buf, sizeof(buf), "%d", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<uint32_t>(const uint32_t &x) {
    char buf[32] = {0,};
    snprintf(buf, sizeof(buf), "%u", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<int64_t>(const int64_t &x) {
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), "%ld", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<long long>(const long long &x) {
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), "%ld", int64_t(x));
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<unsigned long long>(const unsigned long long &x) {
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), "%lu", uint64_t(x));
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<uint64_t>(const uint64_t &x) {
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), "%lu", x);
    std::string res(buf);
    return res;
}

template<>
inline std::string StringUtil::toString<float>(const float &x) {
    StringStreamWrapper oss;
    oss << std::setprecision(6) << x;
    return oss.str();
}

template<>
inline std::string StringUtil::toString<double>(const double &x) {
    StringStreamWrapper oss;
    oss << std::setprecision(15) << x;
    return oss.str();
}

template<typename T> // float or double use snprintf
inline std::string StringUtil::fToString(const T &x, const char *format) {
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), format, x);
    std::string res(buf);
    return res;
}

inline std::string StringUtil::toString(const double &x, int32_t precision) {
    StringStreamWrapper oss;
    oss << std::setprecision(precision) << x;
    return oss.str();
}

template<typename T>
inline std::string StringUtil::toString(const std::vector<T> &x, const std::string &delim) {
    return toString(x.begin(), x.end(), delim);
}

template<typename T>
inline std::string StringUtil::toString(const std::vector<std::string> &x, const std::string &delim) {
    std::string buff;
    for (auto it = x.begin(); it != x.end(); ++it) {
        if (it != x.begin()) buff.append(delim);
        buff.append(*it);
    }
    return buff;
}

template <typename Iterator>
inline std::string StringUtil::toString(Iterator first, Iterator last, const std::string &delim) {
    StringStreamWrapper oss;
    for (Iterator it = first; it != last; ++it) {
        if (it != first) oss << delim;
        oss << toString(*it);
    }
    return oss.str();
}

template<typename T>
inline std::string StringUtil::toString(const std::vector<std::vector<T> > &x,
                                        const std::string &delim1,
                                        const std::string &delim2) {
    std::vector<std::string> strVec;
    for (typename std::vector<std::vector<T> >::const_iterator it = x.begin();
         it != x.end(); ++it)
    {
        strVec.push_back(toString(*it, delim1));
    }
    return toString(strVec, delim2);
}

template<typename T>
inline void StringUtil::fromString(const std::vector<std::string> &strVec, std::vector<T> &vec) {
    vec.clear();
    vec.reserve(strVec.size());
    for (uint32_t i = 0; i < strVec.size(); ++i) {
        vec.push_back(fromString<T>(strVec[i]));
    }
}

template<typename T>
inline void StringUtil::fromString(const std::string &str, std::vector<T> &vec, const std::string &delim) {
    std::vector<std::string> strVec = split(str, delim);
    fromString(strVec, vec);
}

template<typename T>
inline void StringUtil::fromString(const std::string &str, std::vector<std::vector<T> > &vec, const std::string &delim1, const std::string &delim2) {
    vec.clear();
    std::vector<std::string> strVec;
    fromString(str, strVec, delim2);
    vec.resize(strVec.size());
    for (uint32_t i = 0; i < strVec.size(); ++i) {
        fromString(strVec[i], vec[i], delim1);
    }
}

template <typename T>
inline std::string StringUtil::toBinaryString(T x) {
    std::string str;
    str.resize(sizeof(T));
    *((T *)str.data()) = x;
    return str;
}

template <typename T>
bool StringUtil::fromBinaryString(const std::string &str, T &x) {
    if (str.length() != sizeof(T)) {
        return false;
    }
    x = *((T *)str.data());
    return true;
}

template <typename T>
inline std::string StringUtil::getTypeString(const T &t) {
        auto id = typeid(t).name();
        int status;
#if __cplusplus >= 201103L
        std::unique_ptr<char[], void (*)(void*)> result(
                abi::__cxa_demangle(id, 0, 0, &status), std::free);
#else
        std::auto_ptr<char[], void (*)(void*)> result(
                abi::__cxa_demangle(id, 0, 0, &status), std::free);
#endif
        return result.get() ? std::string(result.get()) : "error occurred";
}

inline const std::string& StringUtil::getValueFromMap(const std::string& key,
        const std::map<std::string, std::string>& map)
{
    std::map<std::string, std::string>::const_iterator it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    return NULL_STRING;
}

}

