#pragma once
#include <map>
#include <limits>
#include <vector>
#include <cstring>
#include <algorithm>

#include "autil/MultiValueType.h"
#include "autil/MultiValueCreator.h"
#include "fg_lite/feature/Combiner.h"
#include "fg_lite/feature/Collector.h"
#include "fg_lite/feature/LookupFeatureBTreeEncoder.h"
#include "fg_lite/feature/LookupFeatureDataType.h"

namespace fg_lite {

struct KvUnit {
public:
    KvUnit() {}
    explicit KvUnit(const autil::ConstString &word): word(word.data(),word.size()) {}
    explicit KvUnit(const std::pair<autil::ConstString, std::vector<float>> &pair)
        : word(pair.first.data(), pair.first.size()) , values(pair.second) {}
    explicit KvUnit(const std::string &word, std::vector<float> values):
        word(word), values(values) {}
    explicit KvUnit(std::string &&word, std::vector<float>&& values):
        word(word), values(values) {}

    bool operator< (const KvUnit &rhs) const  {
        return finalHashKey < rhs.finalHashKey;
    }

public:
    static std::vector<KvUnit> getKvUnitVec(
            const std::vector<std::map<autil::ConstString, float>> &rawFields) {
        std::map<autil::ConstString, KvUnit> fields;
        for (const auto &field: rawFields) {
            for (const auto &keyValue: field) {
                if (fields.find(keyValue.first) == fields.end()) {
                    fields[keyValue.first] = KvUnit(keyValue.first);
                }
            }
        }
        for (auto &keyValues: fields) {
            const auto &key = keyValues.first;
            for (const auto &field: rawFields) {
                const auto &value = field.find(key);
                if (value != field.end()) {
                    fields[key].values.push_back(value->second);
                }
                else {
                    fields[key].values.push_back(std::numeric_limits<float>::quiet_NaN());
                }
            }
        }
        std::vector<KvUnit> result;
        for (auto &&keyUnit: std::move(fields)) {
            result.emplace_back(std::move(keyUnit.second));
        }
        return result;
    }

    static std::vector<KvUnit> getKvUnitVec(
            const std::map<autil::ConstString, std::vector<float>> &rawFields,
            uint32_t multiValueDimension) {
        std::vector<KvUnit> result;
        for (const auto &kvs : rawFields) {
            auto kvUnit = KvUnit(kvs.first.toString(), std::vector<float>(multiValueDimension, 0));
            for (auto i = 0; i < kvs.second.size() && i < multiValueDimension; i++) {
                kvUnit.values[i] = kvs.second[i];
            }
            result.emplace_back(std::move(kvUnit));
        }
        return result;
    }

public:
    uint64_t hashKey;
    uint64_t finalHashKey;

    std::string word;
    std::vector<float> values;
};

class LookupFeatureEncoder
{
public:
    LookupFeatureEncoder() = default;
    ~LookupFeatureEncoder() = default;

private:
    LookupFeatureEncoder(const LookupFeatureEncoder &) = delete;
    LookupFeatureEncoder& operator=(const LookupFeatureEncoder &) = delete;

public:

    static size_t getEncodedLength(const std::map<uint64_t, float> &values);
    static bool encodeLegacyV2(const std::map<uint64_t, float> &values, char *buffer, size_t bufferLen);

    static std::string encodeLegacyV2(const std::map<uint64_t, float> &values);
    static bool decodeLegacyV2(const char *buffer, size_t length, uint64_t **key,
                               float **value, size_t *count);

    static bool encodeMultiValue(std::vector<KvUnit> &docs,
                                 const uint32_t dim, std::string &output,
                                 LookupFeatureV3KeyType minHashType = LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
                                 LookupFeatureV3ValueType valueType = LOOKUP_V3_VALUE_ENCODE_AUTO);

    static bool encodeMultiValueBTree(std::vector<KvUnit> &kvUnits,
            const size_t dim, std::string &output,
            LookupFeatureV3KeyType minHashType = LOOKUP_V3_KEY_HASH_0_TO_63_BIT,
            LookupFeatureV3ValueType appointedValueType = LOOKUP_V3_VALUE_ENCODE_AUTO);

    static bool decodeLookupMetadata(const char *data, const size_t dataLength
                              ,const size_t dim, LookupV3Metadata &metadata)
    {
        const char info = data[0];
        const uint8_t headInfo = (int(info) + 256) % 256;
        return validateLookupMetadata(headInfo, dataLength - 1, dim, metadata);
    }


    static bool validateLookupMetadata(const uint8_t headInfo, const size_t dataLength,
            const size_t dim, LookupV3Metadata &metadata) {
        metadata.keyType = (LookupFeatureV3KeyType)(headInfo & 0xfu);
        metadata.keySize = getKeyTypeSize(metadata.keyType);
        metadata.valueType = (LookupFeatureV3ValueType)(headInfo >> 4);
        metadata.valueSize = getValueTypeSize(metadata.valueType);
        const size_t kvUnitSize = metadata.keySize + dim * metadata.valueSize;
        if (dataLength % kvUnitSize != 0) {
            AUTIL_LOG(ERROR, "dataLength is %zu, kvUnitSize is %zu", dataLength, kvUnitSize);
            return false;
        } else {
            metadata.keyCount = dataLength / kvUnitSize;
            return true;
        }
    }

    static bool collectOneField(const autil::ConstString &constStr,
                                std::vector<std::map<autil::ConstString, float>> &mp);
    static bool collectOneField(const autil::MultiString &mts,
                                std::vector<std::map<autil::ConstString, float>> &mp);
public:
    static const std::vector<LookupFeatureV3KeyType> keyHashTryList;
    static const std::vector<LookupFeatureV3ValueType> valueEncodeTryList;
private:
    static constexpr size_t ITEM_SIZE = sizeof(uint64_t) + sizeof(float);
    static constexpr size_t KEY_SIZE = sizeof(uint64_t);
    static constexpr size_t VALUE_SIZE = sizeof(float);
private:
    AUTIL_LOG_DECLARE();
};

}
