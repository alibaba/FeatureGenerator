#pragma once

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif

#if defined(__SSE4_2__)
#  ifndef AUTIL_RAPIDJSON_SSE42
#    define AUTIL_RAPIDJSON_SSE42
#  endif
#elif defined(__SSE2__)
#  ifndef AUTIL_RAPIDJSON_SSE2
#    define AUTIL_RAPIDJSON_SSE2
#  endif
#endif

#include <stddef.h>
#include <stdint.h>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"
#include "rapidjson/error/error.h"

#include "autil/LongHashValue.h"
#include "autil/legacy/exception.h"
#include "autil/legacy/any.h"
#include "autil/legacy/jsonizable_exception.h"

namespace autil {
namespace legacy {

class FastJsonizable;
typedef rapidjson::GenericValue<rapidjson::UTF8<>> RapidValue;
typedef rapidjson::GenericDocument<rapidjson::UTF8<>> RapidDocument;
typedef rapidjson::Writer<rapidjson::StringBuffer> RapidWriter;

template <bool b, class T = void>
using enable_if_t = typename std::enable_if<b, T>::type;

inline std::string FastToJsonString(RapidValue &value);

// serializeToWriter declare begin
inline void serializeToWriter(RapidWriter *writer,
                              const std::string& t);
inline void serializeToWriter(RapidWriter *writer,
                              const char* t);
inline void serializeToWriter(RapidWriter *writer,
                              const FastJsonizable &t);
inline void serializeToWriter(RapidWriter *writer,
                              bool t);
template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t);
template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t);
template <typename T>
inline enable_if_t<std::is_enum<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t);
template <typename T>
inline enable_if_t<std::is_floating_point<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::vector<T>& t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::set<T>& t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::deque<T>& t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::list<T>& t);
template <typename T, typename C>
inline void serializeToWriter(RapidWriter *writer,
                              const std::map<std::string, T, C>& t);
template <typename T, typename U, typename C>
inline void serializeToWriter(RapidWriter *writer,
                              const std::map<T, U, C>& t);
template <typename T, typename U, typename C>
inline void serializeToWriter(RapidWriter *writer,
                              const std::multimap<T, U, C>& t);
template <typename T, typename U>
inline void serializeToWriter(RapidWriter *writer,
                              const std::pair<T, U>& t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const T* t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::shared_ptr<T>& t);
template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::shared_ptr<T>& t);
inline void serializeToWriter(RapidWriter *writer,
                              const ExceptionBase& t);
inline void serializeToWriter(RapidWriter *writer,
                              const uint128_t& t);

// serializeToWriter declare end

// FromRapidValue declare begin
inline void FromRapidValue(FastJsonizable& t, RapidValue& value);
inline void FromRapidValue(std::string& t, RapidValue& value);
inline void FromRapidValue(bool& t, RapidValue& value);
template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, void>
    FromRapidValue(T& t, RapidValue& value);
template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, void>
    FromRapidValue(T& t, RapidValue& value);
template <typename T>
inline enable_if_t<std::is_enum<T>::value, void>
FromRapidValue(T& t, RapidValue& value);
template <typename T>
inline enable_if_t<std::is_floating_point<T>::value, void>
FromRapidValue(T& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(std::vector<T>& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(std::set<T>& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(std::deque<T>& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(std::list<T>& t, RapidValue& value);
template <typename T, typename C>
inline void FromRapidValue(std::map<std::string, T, C>& t, RapidValue& value);
template <typename T, typename U, typename C>
inline void FromRapidValue(std::map<T, U, C>& t, RapidValue& value);
template <typename T, typename U, typename C>
inline void FromRapidValue(std::multimap<T, U, C>& t, RapidValue& value);
template <typename T, typename U>
inline void FromRapidValue(std::pair<T, U>& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(T*& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(std::shared_ptr<T>& t, RapidValue& value);
template <typename T>
inline void FromRapidValue(std::shared_ptr<T>& t, RapidValue& value);
inline void FromRapidValue(ExceptionBase& t, RapidValue& value);
inline void FromRapidValue(uint128_t& t, RapidValue& value);
// FromRapidValue declare end

void FromRapidValue(Any& t, RapidValue& value);
void serializeToWriter(RapidWriter *writer, const Any& a);

// serializeToWriter implemention begin
inline void serializeToWriter(RapidWriter *writer,
                              const std::string& t)
{
    writer->String(t);
}

inline void serializeToWriter(RapidWriter *writer,
                              const char* t)
{
    writer->String(t);
}

inline void serializeToWriter(RapidWriter *writer,
                              bool t)
{
    writer->Bool(t);
}

template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t)
{
    writer->Int64((int64_t)t);
}

template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t)
{
    writer->Uint64((uint64_t)t);
}

template <typename T>
inline enable_if_t<std::is_enum<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t)
{
    writer->Int((int32_t)t);
}

template <typename T>
inline enable_if_t<std::is_floating_point<T>::value, void>
serializeToWriter(RapidWriter *writer,
                  const T& t)
{
    writer->Double((double)t);
}

#define SERIALIZE_AS_ARRAY(container, writer) \
    {                                         \
        writer->StartArray();                 \
        for (auto &v : container) {           \
            serializeToWriter(writer, v);     \
        }                                     \
        writer->EndArray();                   \
    }

template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::vector<T>& t)
{
    SERIALIZE_AS_ARRAY(t, writer)
}

template <>
inline void serializeToWriter<bool>(RapidWriter *writer,
                                    const std::vector<bool>& t)
{
    writer->StartArray();
    for (bool v : t) {
        serializeToWriter(writer, v);
    }
    writer->EndArray();
}

template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::set<T>& t)
{
    SERIALIZE_AS_ARRAY(t, writer)
}

template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::deque<T>& t)
{
    SERIALIZE_AS_ARRAY(t, writer)
}

template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::list<T>& t)
{
    SERIALIZE_AS_ARRAY(t, writer)
}

template <typename T, typename C>
inline void serializeToWriter(RapidWriter *writer,
                              const std::map<std::string, T, C>& t)
{
    writer->StartObject();
    for (auto &kv : t) {
        writer->Key(kv.first.c_str());
        serializeToWriter(writer, kv.second);
    }
    writer->EndObject();
}

template <typename T, typename U, typename C>
inline void serializeToWriter(RapidWriter *writer,
                              const std::map<T, U, C>& t)
{
    writer->StartArray();
    for (auto &kv : t) {
        serializeToWriter(writer, kv);
    }
    writer->EndArray();
}

template <typename T, typename U, typename C>
inline void serializeToWriter(RapidWriter *writer,
                              const std::multimap<T, U, C>& t)
{
    writer->StartArray();
    for (auto &kv : t) {
        serializeToWriter(writer, kv);
    }
    writer->EndArray();
}


template <typename T, typename U>
inline void serializeToWriter(RapidWriter *writer,
                              const std::pair<T, U>& t)
{
    writer->StartArray();
    serializeToWriter(writer, t.first);
    serializeToWriter(writer, t.second);
    writer->EndArray();
}

template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const T* t)
{
    if (t) {
        serializeToWriter(writer, *t);
    } else {
        writer->Null();
    }
}

template <typename T>
inline void serializeToWriter(RapidWriter *writer,
                              const std::shared_ptr<T>& t)
{
    serializeToWriter(writer, t.get());
}

inline void serializeToWriter(RapidWriter *writer,
                              const ExceptionBase& t)
{
    writer->StartObject();
    writer->Key("Message");
    writer->String(t.mMessage);
    writer->Key("File");
    writer->String(t.mFile);
    writer->Key("Function");
    writer->String(t.mFunction);
    writer->Key("Line");
    writer->Int(t.mLine);
    writer->Key("ClassName");
    writer->String(t.GetClassName());
    if (t.mNestedException) {
        writer->Key("Cause");
        serializeToWriter(writer, t.mNestedException);
    }
    writer->EndObject();
}

inline void serializeToWriter(RapidWriter *writer,
                              const uint128_t& t)
{
    writer->StartArray();
    writer->Int64(t.value[0]);
    writer->Int64(t.value[1]);
    writer->EndArray();
}

// serializeToWriter implemention end

// FromRapidValue implemention begin
inline void FromRapidValue(std::string& t, RapidValue& value) {
    if (!value.IsString()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect String but get: " + FastToJsonString(value));
    }
    t = value.GetString();
}

inline void FromRapidValue(bool& t, RapidValue& value) {
    if (!value.IsBool()) {
        // todo: fill more info to exception
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect bool but get: " + FastToJsonString(value));
    }
    t = value.GetBool();
}

template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, void>
FromRapidValue(T& t, RapidValue& value)
{
    if (!value.IsInt64()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect int64 but get: " + FastToJsonString(value));
    }
    t = (T)value.GetInt64();
}

template <typename T>
inline enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, void>
FromRapidValue(T& t, RapidValue& value)
{
    if (!value.IsUint64()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect uint64 but get: " + FastToJsonString(value));
    }
    t = (T)value.GetUint64();
}

template <typename T>
inline enable_if_t<std::is_enum<T>::value, void>
FromRapidValue(T& t, RapidValue& value)
{
    if (!value.IsInt()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect int but get: " + FastToJsonString(value));
    }
    t = (T)value.GetInt();
}

template <typename T>
inline enable_if_t<std::is_floating_point<T>::value, void>
FromRapidValue(T& t, RapidValue& value)
{
    if (!value.IsDouble()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect double but get: " + FastToJsonString(value));
    }
    t = (T)value.GetDouble();
}

#define DESERIALIZE_FROM_ARRAY(container, func, value)                                              \
    {                                                                                               \
        if (!value.IsArray()) {                                                                     \
            AUTIL_LEGACY_THROW(TypeNotMatchException,                                               \
                               "type not match, expect Array but get: " + FastToJsonString(value)); \
        }                                                                                           \
        for (auto itr = value.Begin(); itr != value.End(); ++itr) {                                 \
            T tmpValue;                                                                             \
            FromRapidValue(tmpValue, *itr);                                                         \
            container.func(tmpValue);                                                               \
        }                                                                                           \
    }

template <typename T>
inline void FromRapidValue(std::vector<T>& t, RapidValue& value) {
    DESERIALIZE_FROM_ARRAY(t, push_back, value)
}

template <typename T>
inline void FromRapidValue(std::set<T>& t, RapidValue& value) {
    DESERIALIZE_FROM_ARRAY(t, insert, value)
}

template <typename T>
inline void FromRapidValue(std::deque<T>& t, RapidValue& value) {
    DESERIALIZE_FROM_ARRAY(t, push_back, value)
}

template <typename T>
inline void FromRapidValue(std::list<T>& t, RapidValue& value) {
    DESERIALIZE_FROM_ARRAY(t, push_back, value)
}

template <typename T, typename C>
inline void FromRapidValue(std::map<std::string, T, C>& t, RapidValue& value) {
    if (!value.IsObject()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect Object but get: " + FastToJsonString(value));
    }
    for (auto itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr) {
        std::string k = itr->name.GetString();
        T v;
        FromRapidValue(v, itr->value);
        t.insert(std::make_pair(k, v));
    }
}

template <typename T, typename U, typename C>
inline void FromRapidValue(std::map<T, U, C>& t, RapidValue& value) {
    if (!value.IsArray()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect Array but get: " + FastToJsonString(value));
    }
    for (auto itr = value.Begin(); itr != value.End(); ++itr) {
        std::pair<T, U> tmpPair;
        FromRapidValue(tmpPair, *itr);
        t.insert(tmpPair);
    }
}

template <typename T, typename U, typename C>
inline void FromRapidValue(std::multimap<T, U, C>& t, RapidValue& value) {
    if (!value.IsArray()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect Array but get: " + FastToJsonString(value));
    }
    for (auto itr = value.Begin(); itr != value.End(); ++itr) {
        std::pair<T, U> tmpPair;
        FromRapidValue(tmpPair, *itr);
        t.insert(tmpPair);
    }
}

template <typename T, typename U>
inline void FromRapidValue(std::pair<T, U>& t, RapidValue& value) {
    if (!value.IsArray()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect Array but get: " + FastToJsonString(value));
    }
    if (value.Size() != 2) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect 2 element Array but get: " + FastToJsonString(value));
    }
    FromRapidValue(t.first, value[0]);
    FromRapidValue(t.second, value[1]);
}

template <typename T>
inline void FromRapidValue(T*& t, RapidValue& value) {
    if (value.IsNull()) {
        t = NULL;
        return;
    }
#if __cplusplus >= 201103L
    std::unique_ptr<T> p(new T);
#else
    std::auto_ptr<T> p(new T);
#endif
    FromRapidValue(*p, value);
    t = p.release();
}

template <typename T>
inline void FromRapidValue(std::shared_ptr<T>& t, RapidValue& value) {
    T *p = NULL;
    FromRapidValue(p, value);
    t.reset(p);
}

inline void FromRapidValue(ExceptionBase& t, RapidValue& value) {
    if (!value.IsObject()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect Object but get: " + FastToJsonString(value));
    }
    FromRapidValue(t.mMessage, value["Message"]);
    FromRapidValue(t.mLine, value["Line"]);
    if (value.HasMember("Cause")) {
        FromRapidValue(t.mNestedException, value["Cause"]);
    }
}

inline void FromRapidValue(uint128_t& t, RapidValue& value) {
    if (!value.IsArray()) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect Array but get: " + FastToJsonString(value));
    }
    if (value.Size() != 2) {
        AUTIL_LEGACY_THROW(TypeNotMatchException,
                           "type not match, expect 2 element Array but get: " + FastToJsonString(value));
    }
    FromRapidValue(t.value[0], value[0]);
    FromRapidValue(t.value[1], value[1]);
}

// FromRapidValue implemention end

template <typename T>
std::string FastToJsonString(const T& t) {
    rapidjson::StringBuffer s;
    RapidWriter writer(s);
    serializeToWriter(&writer, t);
    return s.GetString();
}

inline std::string FastToJsonString(RapidValue &value) {
    rapidjson::StringBuffer s;
    RapidWriter writer(s);
    value.Accept(writer);
    return s.GetString();
}

template <typename T>
const char* FastToJsonString(const T& t, rapidjson::StringBuffer &s) {
    RapidWriter writer(s);
    serializeToWriter(&writer, t);
    return s.GetString();
}

template<typename T>
std::string FastToJsonString(const T& t, bool isCompact,
                             const std::string &prefix = "")
{
    // temp for perf test
    return FastToJsonString(t);
}

template <typename T>
void FastFromJsonString(T& t, const std::string& str) {
    RapidDocument document;
    document.Parse(str.c_str());
    if (document.HasParseError()) {
        std::ostringstream errStream;
        errStream << "JSON parse error:" << document.GetParseError()
                  << " offset:" << document.GetErrorOffset();
        AUTIL_LEGACY_THROW(WrongFormatJsonException,
                           errStream.str());
    }
    FromRapidValue(t, document);
}


}
}

