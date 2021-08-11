#pragma once

#include <cmath>
#include "autil/Log.h"

namespace fg_lite {
// 最多为64bit
enum LookupFeatureV3KeyType {
    LOOKUP_V3_KEY_HASH_0_TO_15_BIT = 0,
    LOOKUP_V3_KEY_HASH_16_TO_31_BIT = 1,
    LOOKUP_V3_KEY_HASH_32_TO_47_BIT = 2,
    LOOKUP_V3_KEY_HASH_48_TO_63_BIT = 3,
    LOOKUP_V3_KEY_HASH_0_TO_31_BIT = 4,
    LOOKUP_V3_KEY_HASH_32_TO_63_BIT = 5,
    LOOKUP_V3_KEY_HASH_0_TO_63_BIT = 6
};
// 最多为32bit
enum LookupFeatureV3ValueType {
    LOOKUP_V3_VALUE_ENCODE_8BIT = 0,
    LOOKUP_V3_VALUE_ENCODE_16BIT = 1,
    LOOKUP_V3_VALUE_ENCODE_32BIT = 2, // float32, v2 default
    LOOKUP_V3_VALUE_ENCODE_AUTO = 15  // v3 default
};

struct LookupV3Metadata {
    LookupFeatureV3KeyType keyType;
    LookupFeatureV3ValueType valueType;
    size_t keySize, valueSize;
    size_t keyCount;
};


static const size_t V3_KEY_TYPE_ENCODE_BIT = 4;
constexpr uint8_t getHeadInfo(LookupFeatureV3KeyType keyType, LookupFeatureV3ValueType valueType) {
    return ((valueType << V3_KEY_TYPE_ENCODE_BIT) | keyType);
}


template <size_t N>
struct MatchHashType {
    using HashT = uint64_t;
    static HashT F(uint64_t h) {
        return h;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_0_TO_15_BIT> {
    using HashT = uint16_t;
    static HashT F(uint64_t h) {
        return h & 0xffffu;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_16_TO_31_BIT> {
    using HashT = uint16_t;
    static HashT F(uint64_t h) {
        return (h >> 16) & 0xffffu;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_32_TO_47_BIT> {
    using HashT = uint16_t;
    static HashT F(uint64_t h) {
        return (h >> 32) & 0xffffu;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_48_TO_63_BIT> {
    using HashT = uint16_t;
    static HashT F(uint64_t h) {
        return h >> 48;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_0_TO_31_BIT> {
    using HashT = uint32_t;
    static HashT F(uint64_t h) {
        return h & 0xffffffffu;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_32_TO_63_BIT> {
    using HashT = uint32_t;
    static HashT F(uint64_t h) {
        return h >> 32;
    }
};

template <>
struct MatchHashType<LOOKUP_V3_KEY_HASH_0_TO_63_BIT> {
    using HashT = uint64_t;
    static HashT F(uint64_t h) {
        return h;
    }
};

template <size_t N>
struct MatchValueType {
    using ValueT = uint8_t;
    constexpr static ValueT NOT_FOUND_VALUE = 0xFFu;
    static bool isNotFound(ValueT &value) {
        return value == NOT_FOUND_VALUE;
    }
};

template <>
struct MatchValueType<LOOKUP_V3_VALUE_ENCODE_16BIT> {
    using ValueT = uint16_t;
    constexpr static ValueT NOT_FOUND_VALUE = 0xFFFFu;
    static bool isNotFound(ValueT &value) {
        return value == NOT_FOUND_VALUE;
    }
};

template <>
struct MatchValueType<LOOKUP_V3_VALUE_ENCODE_32BIT> {
    using ValueT = float;
    constexpr static ValueT NOT_FOUND_VALUE = std::numeric_limits<float>::quiet_NaN();
    static bool isNotFound(ValueT &value) {
        return std::isnan(value);
    }
};

#define LOOKUP_V3_MACRO_SWITCH_VALUE_TYPE(MACRO) \
    case LOOKUP_V3_VALUE_ENCODE_8BIT : {         \
        MACRO(LOOKUP_V3_VALUE_ENCODE_8BIT);      \
    }                                            \
    case LOOKUP_V3_VALUE_ENCODE_16BIT : {        \
        MACRO(LOOKUP_V3_VALUE_ENCODE_16BIT);     \
    }                                            \
    case LOOKUP_V3_VALUE_ENCODE_32BIT : {        \
        MACRO(LOOKUP_V3_VALUE_ENCODE_32BIT);     \
    }                                            \
    default:                                     \
    assert(false);

#define LOOKUP_V3_MACRO_SWITCH_KEY_TYPES(MACRO)          \
    case(LOOKUP_V3_KEY_HASH_0_TO_15_BIT): {              \
        MACRO(LOOKUP_V3_KEY_HASH_0_TO_15_BIT); break;    \
    }                                                    \
    case(LOOKUP_V3_KEY_HASH_16_TO_31_BIT): {             \
        MACRO(LOOKUP_V3_KEY_HASH_16_TO_31_BIT); break;   \
    }                                                    \
    case(LOOKUP_V3_KEY_HASH_32_TO_47_BIT): {             \
        MACRO(LOOKUP_V3_KEY_HASH_32_TO_47_BIT); break;   \
    }                                                    \
    case(LOOKUP_V3_KEY_HASH_48_TO_63_BIT): {             \
        MACRO(LOOKUP_V3_KEY_HASH_48_TO_63_BIT); break;   \
    }                                                    \
    case(LOOKUP_V3_KEY_HASH_0_TO_31_BIT): {              \
        MACRO(LOOKUP_V3_KEY_HASH_0_TO_31_BIT); break;    \
    }                                                    \
    case(LOOKUP_V3_KEY_HASH_32_TO_63_BIT): {             \
        MACRO(LOOKUP_V3_KEY_HASH_32_TO_63_BIT); break;   \
    }                                                    \
    case(LOOKUP_V3_KEY_HASH_0_TO_63_BIT): {              \
        MACRO(LOOKUP_V3_KEY_HASH_0_TO_63_BIT); break;    \
    }

#define HASH_KEY_MACRO(TYPE) \
    return MatchHashType<TYPE>::F(value);

inline uint64_t hash(uint64_t value, uint8_t hashType)  {
    switch (hashType) {
        LOOKUP_V3_MACRO_SWITCH_KEY_TYPES(HASH_KEY_MACRO)
    }
    return value;
}

#undef HASH_KEY_MACRO

#define HASH_TYPE_SIZE_MACRO(TYPE) \
    return sizeof(MatchHashType<TYPE>::HashT);

constexpr size_t getKeyTypeSize(LookupFeatureV3KeyType type) {
    switch (type) {
        LOOKUP_V3_MACRO_SWITCH_KEY_TYPES(HASH_TYPE_SIZE_MACRO)
    }
    return sizeof(uint64_t);
}

#undef HASH_TYPE_SIZE_MACRO


#define VALUE_TYPE_SIZE_MACRO(TYPE) \
    return sizeof(MatchValueType<TYPE>::ValueT);

inline size_t getValueTypeSize(LookupFeatureV3ValueType type) {
    switch(type) {
        LOOKUP_V3_MACRO_SWITCH_VALUE_TYPE(VALUE_TYPE_SIZE_MACRO);
    }
    return sizeof(float);
}

#undef VALUE_TYPE_SIZE_MACRO

}
