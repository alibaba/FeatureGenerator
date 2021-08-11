#include "fg_lite/feature/LookupFeatureSparseEncoder.h"

using namespace std;
using namespace autil;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, LookupFeatureSparseEncoder);

#define LOOKUP_SPARSE_ENCODE_CASE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE)     \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {               \
        using HashDataType = typename MatchHashType<KEY_HASH_TYPE>::HashT; \
        using ValueDataType = typename MatchValueType<VALUE_ENCODE_TYPE>::ValueT; \
        map<HashDataType, vector<float>> keyValues;                     \
        for(auto& doc: docs) {                                          \
            const auto &key = doc.first;                                \
            uint64_t hashKey = autil::MurmurHash::MurmurHash64A(key.data(), key.size(), 0); \
            auto finalKey = MatchHashType<KEY_HASH_TYPE>::F(hashKey);   \
            for (const auto& value: doc.second) {                       \
                keyValues[finalKey].push_back((ValueDataType)(value));  \
            }                                                           \
        }                                                               \
        return encodeSparse<HashDataType, ValueDataType>(keyValues, dim, output); \
    }                                                                   \

#define LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(KEY_HASH_TYPE)             \
    LOOKUP_SPARSE_ENCODE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT)    \
    LOOKUP_SPARSE_ENCODE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT)   \
    LOOKUP_SPARSE_ENCODE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)   \

bool LookupFeatureSparseEncoder::encode(
        const map<ConstString, vector<float>> &docs,
        const uint32_t dim,
        string &output,
        LookupFeatureV3KeyType hashType,
        LookupFeatureV3ValueType valueType)
{
    if (docs.empty()) {
        output.clear();
        return true;
    }

    const uint8_t encodeInfo = getHeadInfo(hashType, valueType);
    switch (encodeInfo) {
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_15_BIT)
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT)
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT)
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT)
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT)
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT)
        LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT)
    default:
        AUTIL_LOG(ERROR, "lookup feature encodeSparse hashtype %d and valuetype %d not supported",
                  (int32_t)hashType, (int32_t)valueType);
        return false;
    }
    return true;

#undef LOOKUP_SPARSE_ENCODE_CASE
#undef LOOKUP_SPARSE_ENCODE_CASE_SWITCH_KEY_TYPE

}

}
