#include <algorithm>
#include "autil/MurmurHash.h"
#include "fg_lite/feature/LookupFeatureFunctionV3.h"

using namespace std;
using namespace autil;

namespace fg_lite {


AUTIL_LOG_SETUP(fg_lite, LookupFeatureFunctionV3);

LookupFeatureFunctionV3::LookupFeatureFunctionV3(
        const string &name,
        const Normalizer &normalizer,
        const string &combiner,
        uint32_t dimension,
        const std::vector<float> &boundaries)
    : FeatureFunction(name)
    , _normalizer(normalizer)
    , _combinerType(combinerConvert(combiner))
    , _dimension(dimension)
    , _boundaries(boundaries)
{
}

#define GENERATE_DENSE_FEATURE(type)                                          \
generate<type, MultiDenseFeatures>(&keys, value, features);                   \
    break;

#define GENERATE_INTEGER_FEATURE(type)                                        \
generate<type, MultiIntegerFeatures>(&keys, value, features);                 \
    break;

#define FEATURE_MACRO(MultiFeatureType, MACRO)                                       \
    auto features = new MultiFeatureType(getFeatureName(), typedItemInput->row());   \
    for (uint32_t i = 0; i < typedItemInput->row(); ++i) {                           \
        MultiChar value = typedItemInput->get(i, 0);                                 \
        features->beginDocument();                                                   \
        COMBINER_ENUM(_combinerType, MACRO);                                         \
    }                                                                                \
    return features;

Features *LookupFeatureFunctionV3::genFeatures(
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
        AUTIL_LOG(WARN, "feature[%s], user row count: %d, user data type: %d",
                  getFeatureName().c_str(), (int)userInput->row(), userInput->dataType());
        return nullptr;
    }
    if (itemInput->storageType() != IST_DENSE || itemInput->dataType() != IT_STRING) {
        AUTIL_LOG(ERROR, "feature[%s]: map field must be multichar, actual:IST[%d], IT[%d]",
                  getFeatureName().c_str(), itemInput->storageType(), itemInput->dataType());
        return nullptr;
    }
    typedef FeatureInputTyped<MultiChar, DenseStorage<MultiChar>> FeatureInputTyped;
    FeatureInputTyped *typedItemInput = dynamic_cast<FeatureInputTyped*>(itemInput);
    if (!typedItemInput) {
        return nullptr;
    }

    const auto keys = genKey(userInput);
    if (keys.empty()) {
        AUTIL_LOG(WARN, "no user key");
        return nullptr;
    }

    if (_boundaries.empty()) {
        FEATURE_MACRO(MultiDenseFeatures, GENERATE_DENSE_FEATURE)
    }
    else {
        FEATURE_MACRO(MultiIntegerFeatures, GENERATE_INTEGER_FEATURE)
    }

    return nullptr;
}

#undef GENERATE_DENSE_FEATURE
#undef GENERATE_INTEGER_FEATURE
#undef FEATURE_MACRO

// _dimension:合并的field数;input->row:doc数
template<CombinerType type, typename FeatureT>
void LookupFeatureFunctionV3::generate(const vector<uint64_t> *keys,
                                       const MultiChar &value,
                                       FeatureT *features) const {
    MultiCollectorV3<Combiner<type>, FeatureT> collector(keys, _dimension, &_normalizer,
                                                         &_boundaries, features);
    if (value.size()) {
        match(value, _dimension, collector);
    }
}

#define GEN_KEY_ON_INPUT(input, str, storage)                                       \
auto input = dynamic_cast<const FeatureInputTyped<str, storage<str>>*>(userInput);  \
if (input) {                                                                        \
    for (size_t i = 0; i < input->col(0); i++) {                                    \
        auto value = input->get(0, i);                                              \
        keys.push_back(MurmurHash::MurmurHash64A(value.data(), value.size(), 0));   \
    }                                                                               \
}

vector<uint64_t> LookupFeatureFunctionV3::genKey(const FeatureInput *userInput) const {
    vector<uint64_t> keys;
    GEN_KEY_ON_INPUT(typedInputString, string, DenseStorage);
    GEN_KEY_ON_INPUT(typedInputMultiChar, MultiChar, MultiValueStorage);
    return keys;
}

#undef GEN_KEY_ON_INPUT

template <size_t KeyHashType, size_t ValueEncodeType, typename Collector>
void collectImpl(const char *data, Collector &collector, size_t itemCount, const size_t dim) {
    using HashT = typename MatchHashType<KeyHashType>::HashT;
    using ValueT = typename MatchValueType<ValueEncodeType>::ValueT;
    const auto finalHashFunction = MatchHashType<KeyHashType>::F;
    const auto isNotFound = MatchValueType<ValueEncodeType>::isNotFound;
    const auto keyDataPtr = (HashT *)(data + 1);
    const auto keyDataEnd = keyDataPtr + itemCount;
    const auto valueDataPtr = (ValueT *)(keyDataEnd);
    const auto &keys = collector.getKeys();

    vector<HashT> searchKeys;
    for (const auto& key: keys) {
        searchKeys.emplace_back(finalHashFunction(key));
    }
    sort(searchKeys.begin(), searchKeys.end());

    collector.beginCollect();
    for (const auto& searchKey: searchKeys) {
        const auto currentKeyPtr = std::lower_bound(keyDataPtr, keyDataEnd, searchKey);
        if (keyDataEnd == currentKeyPtr) {
            break;
        }
        if (searchKey == *currentKeyPtr) {
            const auto keyOffset = currentKeyPtr - keyDataPtr;
            for (uint32_t fieldIndex = 0; fieldIndex < dim; ++fieldIndex) {
                ValueT value = valueDataPtr[dim * keyOffset + fieldIndex];
                if (!(isNotFound(value))) {
                    collector.collect(float(value), fieldIndex);
                }
            }
        }
    }
}

#define LOOKUP_V3_MATCH_FEATURE_CASE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE)             \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {                          \
        collectImpl<KEY_HASH_TYPE, VALUE_ENCODE_TYPE>(data.data(), collector,      \
                                                      metadata.keyCount, dim);    \
        break;                                                                     \
    }                                                                              \

#define LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(KEY_HASH_TYPE) \
    LOOKUP_V3_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT)                    \
    LOOKUP_V3_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT)                   \
    LOOKUP_V3_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)

template <typename Collector>
void LookupFeatureFunctionV3::match(const MultiChar &data, const uint32_t dim,
                                    Collector &collector) const
{
    if (data.size() == 0) {
        return;
    }
    LookupV3Metadata metadata;
    if (!LookupFeatureEncoder::decodeLookupMetadata(data.data(), data.size(), dim, metadata)) {
        AUTIL_LOG(WARN, "decode lookup feature buffer failed.");
        return;
    }

    switch(getHeadInfo(metadata.keyType, metadata.valueType)) {
        LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT)
        LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
        LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
    }
}

#undef LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE
#undef LOOKUP_V3_MATCH_FEATURE_CASE
}
