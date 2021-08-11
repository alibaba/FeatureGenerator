#include "fg_lite/feature/LookupFeatureFunctionBTree.h"

using namespace std;
using namespace autil;

namespace fg_lite {

AUTIL_LOG_SETUP(fg_lite, LookupFeatureFunctionBTree);

LookupFeatureFunctionBTree::LookupFeatureFunctionBTree(
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

#define GENERATE_DENSE_FEATURE(type)                                    \
    generate<type, MultiDenseFeatures>(&keys, value, features); \
    break;

#define FEATURE_MACRO(MultiFeatureType, MACRO)                          \
    auto features = new MultiFeatureType(getFeatureName(), typedItemInput->row()); \
    for (uint32_t i = 0; i < typedItemInput->row(); ++i) {              \
        MultiChar value = typedItemInput->get(i, 0);                    \
        features->beginDocument();                                      \
        COMBINER_ENUM(_combinerType, GENERATE_DENSE_FEATURE);           \
    }                                                                   \
    return features;

// with itemInput and userIuput, containing Key/Value typeInfo in the typedItemInput
Features *LookupFeatureFunctionBTree::genFeatures(
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
    typedef FeatureInputTyped<MultiChar, DenseStorage<MultiChar>> FeatureInputTyped;
    FeatureInputTyped *typedItemInput = dynamic_cast<FeatureInputTyped*>(itemInput);
    if (!typedItemInput) {
        AUTIL_LOG(ERROR, "parse item input failed");
        return nullptr;
    }
    auto keys = genKeyFromUserInput<uint64_t>(userInput);
    if (keys.empty()) {
        AUTIL_LOG(WARN, "no user key");
        return nullptr;
    }
    if (_boundaries.empty()) {
        FEATURE_MACRO(MultiDenseFeatures, GENERATE_DENSE_FEATURE)
    } else {
        return nullptr;
    }
}
#undef GENERATE_DENSE_FEATURE
#undef FEATURE_MACRO

#define LOOKUP_BTREE_MATCH_FEATURE_CASE(KEY_HASH_TYPE, VALUE_ENCODE_TYPE)  \
    case getHeadInfo(KEY_HASH_TYPE, VALUE_ENCODE_TYPE): {               \
        doBranchFind<KEY_HASH_TYPE, VALUE_ENCODE_TYPE>(data, collector, \
                _dimension);                                \
        break;                                                          \
    }

#define LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(KEY_HASH_TYPE)        \
    LOOKUP_BTREE_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT) \
    LOOKUP_BTREE_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT) \
    LOOKUP_BTREE_MATCH_FEATURE_CASE(KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT)

template<CombinerType type, typename FeatureT>
void LookupFeatureFunctionBTree::generate(
        const vector<uint64_t> *keys,
        const MultiChar &data,
        FeatureT *features) const
{
    if (data.size()) {
        MultiCollectorV3<Combiner<type>, FeatureT> collector(keys, _dimension, &_normalizer,&_boundaries, features);
        LookupV3Metadata metadata;
        const uint8_t headInfo = (*((headInfoStruct*)data.data())).typeInfo;
        if (!LookupFeatureEncoder::validateLookupMetadata(headInfo, data.size() - sizeof(headInfoStruct),
                        _dimension, metadata))
        {
            AUTIL_LOG(WARN, "decode lookup feature buffer failed.");
            return;
        }
        switch(getHeadInfo(metadata.keyType, metadata.valueType)) {
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE (LOOKUP_V3_KEY_HASH_0_TO_15_BIT);
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT);
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT);
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT);
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE (LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
            LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE (LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
        }
    } else {
        AUTIL_LOG(INFO, "generate data parameter empty");
    }
}
#undef LOOKUP_MATCH_FEATURE_CASE_SWITCH_KEY_TYPE
#undef LOOKUP_BTREE_MATCH_FEATURE_CASE

} // fg_lite
