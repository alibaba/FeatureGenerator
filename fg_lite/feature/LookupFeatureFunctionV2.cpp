#include "fg_lite/feature/LookupFeatureFunctionV2.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

using namespace std;
using namespace autil;

namespace fg_lite {


AUTIL_LOG_SETUP(fg_lite, LookupFeatureFunctionV2);

LookupFeatureFunctionV2::LookupFeatureFunctionV2(
        const string &name,
        const Normalizer &normalizer,
        const string &combiner,
        uint32_t dimension,
        const std::vector<float> &boundaries,
        bool useHeader,
        bool useSparse,
        LookupFeatureV3KeyType keyType,
        LookupFeatureV3ValueType valueType)
    : FeatureFunction(name)
    , _normalizer(normalizer)
    , _combinerType(combinerConvert(combiner))
    , _dimension(dimension)
    , _boundaries(boundaries)
    , _useHeader(useHeader)
    , _useSparse(useSparse)
    , _keyType(keyType)
    , _valueType(valueType)
{
}

Features *LookupFeatureFunctionV2::genFeatures(
        const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "input count invalid, expected[%d], actual[%d]",
                  (int)getInputCount(), (int)inputs.size());
        return nullptr;
    }

    FeatureInput *itemInput = inputs[0];
    FeatureInput *userInput = inputs[1];
    if (userInput->row() != 1) {
        AUTIL_LOG(ERROR, "feature[%s], user row count: %d, user data type: %d",
                  getFeatureName().c_str(), (int)userInput->row(), userInput->dataType());
        return nullptr;
    }

    if (itemInput->storageType() != IST_DENSE
        || itemInput->dataType() != IT_STRING)
    {
        AUTIL_LOG(ERROR,
                  "feature[%s]: map field must be multichar, actual:IST[%d], IT[%d]",
                  getFeatureName().c_str(),
                  itemInput->storageType(),
                  itemInput->dataType());
        return nullptr;
    }

    typedef FeatureInputTyped<MultiChar, DenseStorage<MultiChar>> FeatureInputTyped;
    FeatureInputTyped *typedItemInput = dynamic_cast<FeatureInputTyped*>(itemInput);
    if (!typedItemInput) {
        AUTIL_LOG(ERROR, "parse item input failed");
        return nullptr;
    }

    vector<uint64_t> keys;
    keys = genKey(userInput);
    if (keys.empty()) {
        AUTIL_LOG(WARN, "no user key");
        return nullptr;
    }

    sort(keys.begin(), keys.end());
    if (_dimension == 1) {
        if (_boundaries.empty()) {
            auto features = new SingleDenseFeatures(getFeatureName(), typedItemInput->row());
#define GENERATE_FEATURE(type)                                          \
            generate<type, SingleDenseFeatures>(&keys, typedItemInput, features); \
            break;

            COMBINER_ENUM(_combinerType, GENERATE_FEATURE);
#undef GENERATE_FEATURE
            return features;
        } else {
            auto features = new SingleIntegerFeatures(getFeatureName(), typedItemInput->row());
#define GENERATE_FEATURE(type)                                          \
            generate<type, SingleIntegerFeatures>(&keys, typedItemInput, features); \
            break;

            COMBINER_ENUM(_combinerType, GENERATE_FEATURE);
#undef GENERATE_FEATURE
            return features;
        }
    } else {
        MultiDenseFeatures *features = new MultiDenseFeatures(
                getFeatureName(), typedItemInput->row() * _dimension);
#define GENERATE_FEATURE(type)                                          \
        generateMultiDense<type>(&keys, typedItemInput, features, _dimension); \
        break;

        COMBINER_ENUM(_combinerType, GENERATE_FEATURE);
#undef GENERATE_FEATURE
        return features;
    }
}

template<CombinerType type, typename FeatureT>
void LookupFeatureFunctionV2::generate(const vector<uint64_t> *keys,
                                       FeatureInputTyped<MultiChar, DenseStorage<MultiChar>> *input,
                                       FeatureT *features) const
{
    for (uint32_t i = 0; i < input->row(); ++i) {
        SingleCollector<Combiner<type>, FeatureT> collector(keys, &_normalizer, &_boundaries, features);
        MultiChar value = input->get(i, 0);
        if (_useHeader) {
            matchWithHeader(value, collector);
        } else {
            match(value.data(), value.size(), collector);
        }
    }
}

template <CombinerType type>
void LookupFeatureFunctionV2::generateMultiDense(const vector<uint64_t> *keys,
        FeatureInputTyped<MultiChar, DenseStorage<MultiChar>> *input,
        MultiDenseFeatures *features, uint32_t dimension) const
{
    // keys must be sorted
    features->_featureValues.resize(dimension * input->row(), 0.0f);
    for (uint32_t i = 0; i < input->row(); ++i) {
        uint32_t offset = dimension * i;
        features->_offsets.push_back(offset);
        MultiChar value = input->get(i,0);
        float *buffer = features->_featureValues.data() + offset;
        MultiDimensionCollector<type> collector(keys, buffer, dimension);
        if (_useSparse) {
            matchSparseEncode(value, collector, dimension);
        } else if (_useHeader) {
            matchWithHeader(value, collector, dimension);
        } else {
            match(value.data(), value.size(), collector);
        }

    }
}

vector<uint64_t> LookupFeatureFunctionV2::genKey(FeatureInput *userInput) const {
    vector<uint64_t> keys;
    auto typedInput = dynamic_cast<FeatureInputTyped<string, DenseStorage<string>>*>(userInput);
    if (typedInput) {
        for (size_t i = 0; i < typedInput->col(0); i++) {
            const auto value = typedInput->get(0, i);
            keys.push_back(MurmurHash::MurmurHash64A(value.data(), value.size(), 0));
        }
    }
    auto typedInput1 = dynamic_cast<FeatureInputTyped<MultiChar, MultiValueStorage<MultiChar>>*>(userInput);
    if (typedInput1) {
        for (size_t i = 0; i < typedInput1->col(0); i++) {
            auto value = typedInput1->get(0, i);
            keys.push_back(MurmurHash::MurmurHash64A(value.data(), value.size(), 0));
        }
    }
    return keys;
}

template<LookupFeatureV3KeyType KeyHashType,
         LookupFeatureV3ValueType ValueEncodeType,
         typename Collector>
void collectImpl(const char *data, Collector &collector, size_t keyNum,
                 const size_t dimension)
{
    using HashT = typename MatchHashType<KeyHashType>::HashT;
    using ValueT = typename MatchValueType<ValueEncodeType>::ValueT;
    const auto finalHashFunction = MatchHashType<KeyHashType>::F;
    const auto keyDataPtr = (HashT *)(data + 1);
    const auto keyDataEnd = keyDataPtr + keyNum;
    const auto valueDataPtr = (ValueT *)(keyDataEnd);
    float buffer[dimension];
    while (!collector.end()) {
        const auto &key = collector.getKey();
        const auto searchKey = finalHashFunction(key);
        const auto currentKeyPtr = std::lower_bound(keyDataPtr, keyDataEnd, searchKey);
        if (keyDataEnd == currentKeyPtr) {
            break;
        }
        if (searchKey == *currentKeyPtr) {
            auto start = valueDataPtr + (currentKeyPtr - keyDataPtr) * dimension;
            auto end = start + dimension;
            std::transform(start, end, buffer, [](ValueT v) {return (float)v;});
            collector.collect(buffer);
        }
    }
}

template<LookupFeatureV3KeyType KeyHashType,
         LookupFeatureV3ValueType ValueEncodeType,
         typename Collector>
void collectSparseEncode(const char* data, size_t dataLen, Collector &collector,
                         const size_t dimension)
{
    using KeyType = typename MatchHashType<KeyHashType>::HashT;
    using ValueType = typename MatchValueType<ValueEncodeType>::ValueT;
    const auto finalHashFunction = MatchHashType<KeyHashType>::F;

    SparseDecoder<KeyType, ValueType> decoder(data, dimension);

    float values[dimension];
    memset( values, 0.0f, dimension*sizeof(float) );
    while (!collector.end()) {
        uint64_t key = collector.getKey();
        const auto searchKey = finalHashFunction(key);
        if (decoder.find(searchKey, values, dimension)) {
            collector.collect(values);
        }
    }
}

#define LOOKUP_MATCH_SPARSE_ENCODE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE)    \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {               \
        collectSparseEncode<KEY_HASH_TYPE, VALUE_ENCODE_TYPE>(data.data(), data.size(), \
                collector, dim);                                        \
        break;                                                          \
    }

#define LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(KEY_HASH_TYPE)       \
    LOOKUP_MATCH_SPARSE_ENCODE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT) \
    LOOKUP_MATCH_SPARSE_ENCODE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT) \
    LOOKUP_MATCH_SPARSE_ENCODE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)

template<typename Collector>
void LookupFeatureFunctionV2::matchSparseEncode(const MultiChar &data,
        Collector &collector, uint32_t dim) const {
    if (data.size() == 0) {
        return;
    }
    switch(getHeadInfo(_keyType, _valueType)) {
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_15_BIT);
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT);
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT);
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT);
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
        LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
    default:
        AUTIL_LOG(ERROR, "head info not match");
    }
}

#undef LOOKUP_MATCH_SPARSE_ENCODE_SWITCH_KEY_TYPE
#undef LOOKUP_MATCH_SPARSE_ENCODE

#define LOOKUP_V2_MATCH_FEATURE_CASE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE)             \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {                          \
        collectImpl<KEY_HASH_TYPE, VALUE_ENCODE_TYPE>(data.data(), collector,      \
                                                      metadata.keyCount, dim);    \
        break;                                                                     \
    }                                                                              \

#define LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(KEY_HASH_TYPE) \
    LOOKUP_V2_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT)                    \
    LOOKUP_V2_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT)                   \
    LOOKUP_V2_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)

template<typename Collector>
void LookupFeatureFunctionV2::matchWithHeader(const MultiChar &data,
        Collector &collector, uint32_t dim)
{
    if (data.size() == 0) {
        return;
    }
    LookupV3Metadata metadata;
    if (!LookupFeatureEncoder::decodeLookupMetadata(data.data(), data.size(),
                    dim, metadata))
    {
        AUTIL_LOG(WARN, "decode lookup feature buffer failed.");
        return;
    }

    switch(getHeadInfo(metadata.keyType, metadata.valueType)) {
        LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
        LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
        LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
    }
}

#undef LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE
#undef LOOKUP_V2_MATCH_FEATURE_CASE


template<typename Collector>
void LookupFeatureFunctionV2::match(const char *data, size_t len,
                                    Collector &collector) const
{
    uint64_t *keyBuffer = nullptr;
    float *valueBuffer = nullptr;
    size_t itemCount = 0;
    if (!LookupFeatureEncoder::decodeLegacyV2(data, len, &keyBuffer, &valueBuffer, &itemCount)) {
        AUTIL_LOG(WARN, "decode buffer failed");
        return;
    }

    uint64_t *keyBufferEnd = keyBuffer + itemCount;
    uint64_t *current = keyBuffer;
    while (!collector.end()) {
        auto key = collector.getKey();
        current = lower_bound(current, keyBufferEnd, key);
        if (keyBufferEnd == current) {
            break;
        }
        // in case same key, don't current++;
        if (key == *current) {
            collector.collect(valueBuffer + (current - keyBuffer));
        }
    }
}



}
