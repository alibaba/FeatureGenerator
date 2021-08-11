#include "fg_lite/feature/IdFeatureFunction.h"
#include "autil/StringUtil.h"
#include <algorithm>

using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, IdFeatureFunction);

IdFeatureFunction::IdFeatureFunction(const string &name,
                                     const string &prefix,
                                     int32_t pruneTo,
                                     const vector<string> &invalidValues)
    : FeatureFunction(name, prefix)
    , _pruneTo(pruneTo)
    , _invalidValues(invalidValues)
{
    autil::StringUtil::fromString(_invalidValues, _intInvalidValues);
    autil::StringUtil::fromString(_invalidValues, _floatInvalidValues);
}

Features *IdFeatureFunction::genFeatures(const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "IdFeature[%s] inputs size not equal 1", getFeatureName().c_str());
        return nullptr;
    }
    FeatureInput *input = inputs[0];
#define CASE(t)                                                         \
    case t: {                                                           \
        typedef InputType2Type<t>::Type Type;                           \
        if (input->storageType() == IST_DENSE) {                        \
            return genFeatures<Type, DenseStorage<Type>>(input);        \
        } else if (input->storageType() == IST_SPARSE_MULTI_VALUE) {    \
            return genFeatures<Type, MultiValueStorage<Type>>(input);   \
        } else {                                                        \
            return genFeatures<Type, ValueOffsetStorage<Type>>(input);  \
        }                                                               \
    }                                                                   \
        break
    switch (input->dataType()) {
        INPUT_DATA_TYPE_MACRO_HELPER(CASE);
    default:
        AUTIL_LOG(ERROR, "IdFeature[%s] input type[%d] not support",
                  getFeatureName().c_str(), input->dataType());
        return nullptr;
    }
#undef CASE
};

template <typename T, typename StorageType>
Features *IdFeatureFunction::genFeatures(FeatureInput *input) const {
    typedef FeatureInputTyped<T, StorageType> FeatureInputTyped;
    FeatureInputTyped *typedInput = dynamic_cast<FeatureInputTyped*>(input);
    if (!typedInput) {
        return nullptr;
    }

    MultiSparseFeatures *features = new MultiSparseFeatures(typedInput->row());
#define GEN_FEATURES(fun)                               \
    for (size_t i = 0; i < typedInput->row(); i++) {    \
        features->beginDocument();                      \
        fun(typedInput, features, i);                   \
    }                                                   \

    GEN_FEATURES(genSimpleFeatures);

    return features;
}

template <typename TypedFeatureInput>
void IdFeatureFunction::genSimpleFeatures(TypedFeatureInput *input, MultiSparseFeatures *features, int row) const {
    for (size_t j = 0; j < min(input->col(row), size_t(_pruneTo)); j++) {
        auto value = input->get(row, j);
        if (FeatureFormatter::isInvalidValue(value) || isInvalid(value)) {
            continue;
        }
        FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
        FeatureFormatter::fillFeatureToBuffer(value, buffer);
        features->addFeatureKey(buffer.data(), buffer.size());
    }
}

}
