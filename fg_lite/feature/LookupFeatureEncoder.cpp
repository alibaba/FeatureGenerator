#include <limits>
#include <set>
#include "autil/StringUtil.h"
#include "autil/MurmurHash.h"
#include "autil/StringTokenizer.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

using namespace std;
using autil::StringUtil;
using autil::ConstString;
using autil::MultiString;
using autil::StringTokenizer;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, LookupFeatureEncoder);

const std::vector<LookupFeatureV3KeyType> LookupFeatureEncoder::keyHashTryList = {
    LOOKUP_V3_KEY_HASH_0_TO_15_BIT,
    LOOKUP_V3_KEY_HASH_16_TO_31_BIT,
    LOOKUP_V3_KEY_HASH_32_TO_47_BIT,
    LOOKUP_V3_KEY_HASH_48_TO_63_BIT,
    LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
    LOOKUP_V3_KEY_HASH_32_TO_63_BIT,
    LOOKUP_V3_KEY_HASH_0_TO_63_BIT
};

size_t LookupFeatureEncoder::getEncodedLength(const map<uint64_t, float> &values) {
    return ITEM_SIZE * values.size();
}

string LookupFeatureEncoder::encodeLegacyV2(const std::map<uint64_t, float> &values) {
    auto size = getEncodedLength(values);
    string data;
    data.resize(size);
    encodeLegacyV2(values, (char*)data.data(), data.size());
    return data;
}

bool LookupFeatureEncoder::encodeLegacyV2(const std::map<uint64_t, float> &values,
        char *buffer, size_t bufferLen)
{
    size_t expectedLength = getEncodedLength(values);
    if (bufferLen != expectedLength) {
        return false;
    }
    uint64_t *keyBuffer = (uint64_t *)buffer;
    float *valueBuffer = (float *)(keyBuffer + values.size());
    for (const auto &it : values) {
        *keyBuffer++ = it.first;
        *valueBuffer++ = it.second;
    }
    return true;
}

bool LookupFeatureEncoder::decodeLegacyV2(const char *buffer, size_t length,
        uint64_t **key, float **value, size_t *count)
{
    if (length % ITEM_SIZE != 0) {
        return false;
    }
    *count = length / ITEM_SIZE;
    *key = (uint64_t *)buffer;
    *value = (float *)(*key + *count);
    return true;
}

LookupFeatureV3ValueType getValueEncodeType(vector<KvUnit> kvUnits) {
    float maxMagnitude = std::numeric_limits<float>::lowest();
    for (const auto &kvUnit: kvUnits) {
        maxMagnitude = std::max(maxMagnitude,
                                fabs(*std::max_element(kvUnit.values.cbegin(), kvUnit.values.cend(),
                                                [](const float& lhs, const float& rhs) {
                                                    return fabs(lhs) < fabs(rhs);
                                                } )) );
    }
    // note: the value is of type float but it only contains integer.
    if (maxMagnitude < 255) {
        return LOOKUP_V3_VALUE_ENCODE_8BIT;
    } else if (maxMagnitude < 65535) {
        return LOOKUP_V3_VALUE_ENCODE_16BIT;
    } else {
        return LOOKUP_V3_VALUE_ENCODE_32BIT; // float
    }
}

LookupFeatureV3KeyType tryHashKvUnitKeys(vector<KvUnit> &kvUnits, LookupFeatureV3KeyType minHashType) {
    const auto &count = kvUnits.size();
    for (const auto keyHashType : LookupFeatureEncoder::keyHashTryList) {
        if (keyHashType < minHashType) {
            continue;
        }
        std::set<uint64_t> hashedKeys;
        for (auto& kvUnit : kvUnits) {
            auto hashedKey = hash(kvUnit.hashKey, keyHashType);
            kvUnit.finalHashKey = hashedKey;
            hashedKeys.insert(hashedKey);
        }
        if (hashedKeys.size() == count) {
            return keyHashType;
        }
    }
    return LOOKUP_V3_KEY_HASH_0_TO_63_BIT; //e.g. uint64_t: 6;
}


// encode format: k1 k2 k3 | v11v12 v21v22 v31v32
template <size_t KeyType, size_t ValueType>
bool encodeLookupV3Data(const std::vector<KvUnit> &kvUnits, const size_t dim,
                        std::string &encodedData) noexcept
{
    using HashDataType = typename MatchHashType<KeyType>::HashT;
    using ValueDataType = typename MatchValueType<ValueType>::ValueT;
    const auto notFoundValue = MatchValueType<ValueType>::NOT_FOUND_VALUE;
    const auto kvPairEncodeSize = sizeof(HashDataType) + dim * sizeof(ValueDataType);
    encodedData.resize(1 + kvUnits.size() * kvPairEncodeSize, 0);
    auto dataPtr = (char *)encodedData.data();
    *dataPtr = getHeadInfo((LookupFeatureV3KeyType)KeyType, (LookupFeatureV3ValueType)ValueType);
    dataPtr++;
    HashDataType* keyData = (HashDataType *)dataPtr;
    ValueDataType* valueData = (ValueDataType *)(keyData + kvUnits.size());
    for (size_t idx = 0; idx < kvUnits.size(); ++idx) {
        keyData[idx] = (HashDataType)kvUnits[idx].finalHashKey;
        ValueDataType* valueSegment = valueData + dim * idx;
        for (size_t fieldIndex = 0; fieldIndex < dim; ++fieldIndex) {
            const auto value = kvUnits[idx].values[fieldIndex];
            valueSegment[fieldIndex] = std::isnan(value) ? notFoundValue : (ValueDataType)value;
        }
    }
    return true;
}

template <LookupFeatureV3KeyType KeyType, LookupFeatureV3ValueType ValueType>
static bool encodeLookupBTreeData(const std::vector<KvUnit> &kvUnits,
                                  std::string &encodedData) noexcept
{
    using HashDataType = typename MatchHashType<KeyType>::HashT;
    using ValueDataType = typename MatchValueType<ValueType>::ValueT;
    const auto notFoundValue = MatchValueType<ValueType>::NOT_FOUND_VALUE;
    using vecValueType = vector<ValueDataType>;
    std::map<HashDataType, vecValueType> doc;
    const size_t dim = kvUnits[0].values.size(); // kvUnits isn't empty;
    for (auto& entity : kvUnits) {
        HashDataType key = (HashDataType)entity.finalHashKey;
        for (auto val : entity.values) {
            if (std::isnan(val)) {
                doc[key].push_back(notFoundValue);
            } else {
                doc[key].push_back((ValueDataType)val);
            }
        }
    }
    encodedData = LookupFeatureBTreeEncoder::encode<HashDataType, ValueDataType>(doc, 8, dim, KeyType, ValueType);
    return true;
}

#define LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE) \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {               \
        return encodeLookupV3Data<KEY_HASH_TYPE, VALUE_ENCODE_TYPE>(kvUnits, dim, output); \
    }

#define LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(KEY_HASH_TYPE)     \
    LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT) \
    LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT) \
    LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)

bool LookupFeatureEncoder::encodeMultiValue(vector<KvUnit> &kvUnits, const uint32_t dim,
        string &output, LookupFeatureV3KeyType minHashType, LookupFeatureV3ValueType appointedValueType)
{
    if (kvUnits.empty()) {
        output.clear();
        return true;
    }

    for (auto& kvUnit: kvUnits) {
        kvUnit.hashKey = autil::MurmurHash::MurmurHash64A(kvUnit.word.data(),kvUnit.word.size(), 0);
    }

    const auto keyHashType = tryHashKvUnitKeys(kvUnits, minHashType);
    std::sort(kvUnits.begin(), kvUnits.end());

    const auto valueType = (appointedValueType == LOOKUP_V3_VALUE_ENCODE_AUTO ?
                            getValueEncodeType(kvUnits) : appointedValueType); // auto value:15; (v3 default)

    const uint8_t headInfo = getHeadInfo(keyHashType, valueType); // get a head info number
    switch (headInfo) {
        LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE (LOOKUP_V3_KEY_HASH_0_TO_31_BIT)
            LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT)
            LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE (LOOKUP_V3_KEY_HASH_0_TO_63_BIT)

            LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE (LOOKUP_V3_KEY_HASH_0_TO_15_BIT)
            LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT)
            LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT)
            LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT)

    default:
            AUTIL_LOG(ERROR, "lookup feature v3 kv encode type [%d] and [%d] not supported",
                      (int32_t)keyHashType, (int32_t)valueType);
        return false;
    }

    return true;
}

#undef LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE
#undef LOOKUP_V3_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE

#define LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE) \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {               \
        return encodeLookupBTreeData<KEY_HASH_TYPE, VALUE_ENCODE_TYPE>(kvUnits, output); \
    }

#define LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(KEY_HASH_TYPE)  \
    LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT) \
    LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT) \
    LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)

bool LookupFeatureEncoder::encodeMultiValueBTree(
        std::vector<KvUnit> &kvUnits,
        const size_t dim, std::string &output,
        LookupFeatureV3KeyType minHashType,
        LookupFeatureV3ValueType appointedValueType) {

    if (kvUnits.empty()) {
        output.clear();
        AUTIL_LOG(INFO, "lookup feature encodeMultiValueBTree kvUnits parameter empty.");
        return true;
    }
    for (auto& kvUnit: kvUnits) {
        kvUnit.hashKey = autil::MurmurHash::MurmurHash64A(kvUnit.word.data(),kvUnit.word.size(), 0);
    }
    const auto keyHashType = tryHashKvUnitKeys(kvUnits, minHashType); // e.g. keyHashType = 6
    std::sort(kvUnits.begin(), kvUnits.end()); // hashed accordingly
    const auto valueType = (appointedValueType == LOOKUP_V3_VALUE_ENCODE_AUTO) ?
                           getValueEncodeType(kvUnits) : appointedValueType;
    const uint8_t headInfo = getHeadInfo(keyHashType, valueType); // get a head info number
    switch (headInfo) {
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT)
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT)
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT)
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_15_BIT)
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT)
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT)
            LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT)
    default:
            AUTIL_LOG(ERROR, "lookup feature btree kv encode type [%d] and [%d] not supported",
                      (int32_t)keyHashType, (int32_t)valueType);
        return false;
    }
    return true;
}
#undef LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_VALUE_TYPE
#undef LOOKUP_BTREE_MEMORY_ENCODE_CASE_SWITCH_KEY_TYPE

bool stringToKvImpl(const ConstString &kvStr, map<ConstString, float> &mp) {
    size_t pos = kvStr.rfind(':');
    if (pos == string::npos) {
        return false;
    }
    float tmpValue;
    auto key = ConstString(kvStr.data(), pos);
    auto subStr = kvStr.substr(pos+1);
    string stdStr(subStr.c_str(), subStr.length());
    if(!StringUtil::strToFloat(stdStr.c_str(), tmpValue) || fabs(tmpValue) < 1E-6) {
        return false;
    }
    mp.insert({key, tmpValue});
    return true;
}

bool LookupFeatureEncoder::collectOneField(
        const MultiString &mts
        , vector<map<ConstString, float>> &mp) {
    map<ConstString, float> tmp;
    for (size_t i = 0; i < mts.size(); ++i) {
        ConstString kvStr(mts[i].data(), mts[i].size());
        if(!stringToKvImpl(kvStr, tmp)) {
            AUTIL_LOG(ERROR, "LookupFeatureEncoder::stringToKvImpl failed.");
            return false;
        }
    }
    mp.emplace_back(std::move(tmp));
    return true;
}

bool LookupFeatureEncoder::collectOneField(
        const ConstString &constStr
        , vector<map<ConstString, float>> &mp) {
    map<ConstString, float> tmp;
    if (constStr.size() == 0) {
        mp.emplace_back(std::move(tmp));
        return true;
    }
    auto kvStrVec = StringTokenizer::constTokenize(constStr, autil::MULTI_VALUE_DELIMITER, StringTokenizer::TOKEN_LEAVE_AS);
    for (const auto &kvStr: kvStrVec) {
        stringToKvImpl(kvStr, tmp);
    }
    mp.emplace_back(std::move(tmp));
    return true;
}


}
