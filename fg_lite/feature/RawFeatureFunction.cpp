#include "fg_lite/feature/RawFeatureFunction.h"
#include "fg_lite/feature/FloatValueConvertor.h"

using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, RawFeatureFunction);

Features *RawFeatureFunction::genFeatures(
        const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "RawFeature[%s] inputs size not equal 1", getFeatureName().c_str());
        return nullptr;
    }
    FeatureInput *input = inputs[0];
    if (input->row() == 0) {
        return nullptr;
    }
    if (input->storageType() == IST_DENSE && input->col(0) == 1) {
        if (_boundaries.empty()) {
            return genDenseFeatures<SingleDenseFeatures>(input);
        } else {
            return genDenseFeatures<SingleIntegerFeatures>(input);
        }
    } else {
        if (_boundaries.empty()) {
            return genSparseFeatures<MultiDenseFeatures>(input);
        } else {
            return genSparseFeatures<MultiIntegerFeatures>(input);
        }
    }
}

template <typename T, typename FeatureType, typename StorageType>
void RawFeatureFunction::genFeaturesTyped(
        FeatureInput *input,
        FeatureType *features) const
{
    typedef FeatureInputTyped<T, StorageType> FeatureInputTyped;
    FeatureInputTyped *typedInput = dynamic_cast<FeatureInputTyped*>(input);
    for (size_t i = 0; i < typedInput->row(); i++) {
        appendSparseOffset(features);
        for (size_t j = 0; j < typedInput->col(i); j++) {
            T value = typedInput->get(i, j);
            float floatValue = 0.0f;
            FloatValueConvertor::convertToFloat(value, floatValue);
            if (std::isnan(floatValue)) {
                floatValue = 0.0f;
            }
            floatValue = _normalizer.normalize(floatValue);
            addFeatureMayBucketize(features, _boundaries, floatValue);
        }
        if (!_boundaries.empty()) {
            for (int j = typedInput->col(i); j < _valueDimension; j++) {
                addFeatureMayBucketize(features, _boundaries, 0);
            }
        }
    }
}

template<typename FeatureType>
Features *RawFeatureFunction::genDenseFeatures(FeatureInput *input) const {
    auto features = make_unique<FeatureType>(getFeatureName(), input->row());

#define CASE(t)                                                         \
    case t: {                                                           \
        typedef InputType2Type<t>::Type Type;                           \
        genFeaturesTyped<Type, FeatureType, DenseStorage<Type> >(input, features.get()); \
    }                                                                   \
        break
    switch (input->dataType()) {
        CASE(IT_INT8);
        CASE(IT_UINT8);
        CASE(IT_INT16);
        CASE(IT_UINT16);
        CASE(IT_INT32);
        CASE(IT_UINT32);
        CASE(IT_INT64);
        CASE(IT_UINT64);
        CASE(IT_FLOAT);
        CASE(IT_DOUBLE);
        CASE(IT_STRING);
        CASE(IT_CSTRING);
    default:
        AUTIL_LOG(ERROR, "RawFeature[%s] input type[%d] not support",
                  getFeatureName().c_str(), input->dataType());
        return nullptr;
    }
#undef CASE
    return features.release();
}

template<typename FeatureType>
Features *RawFeatureFunction::genSparseFeatures(FeatureInput *input) const {
    auto features = make_unique<FeatureType>(getFeatureName(), input->row());

#define CASE(t)                                                         \
    case t: {                                                           \
        typedef InputType2Type<t>::Type Type;                           \
        if (input->storageType() == IST_SPARSE_MULTI_VALUE) {           \
            genFeaturesTyped<Type, FeatureType, MultiValueStorage<Type> >(input, features.get()); \
        } else if (input->storageType() == IST_DENSE) {                              \
            genFeaturesTyped<Type, FeatureType, DenseStorage<Type> >(input, features.get()); \
        } else {                                                        \
            genFeaturesTyped<Type, FeatureType, ValueOffsetStorage<Type> >(input, features.get()); \
        }                                                               \
    }                                                                   \
        break
    switch (input->dataType()) {
        CASE(IT_INT8);
        CASE(IT_UINT8);
        CASE(IT_INT16);
        CASE(IT_UINT16);
        CASE(IT_INT32);
        CASE(IT_UINT32);
        CASE(IT_INT64);
        CASE(IT_UINT64);
        CASE(IT_FLOAT);
        CASE(IT_DOUBLE);
        CASE(IT_STRING);
        CASE(IT_CSTRING);
    default:
        AUTIL_LOG(ERROR, "RawFeature[%s] input type[%d] not support",
                  getFeatureName().c_str(), input->dataType());
        return nullptr;
    }
#undef CASE
    return features.release();
}

template<typename Features>
void RawFeatureFunction::appendSparseOffset(Features *features) {
}

template<>
void RawFeatureFunction::appendSparseOffset<MultiDenseFeatures>(MultiDenseFeatures *features) {
    features->_offsets.push_back(features->_featureValues.size());
}

template<>
void RawFeatureFunction::appendSparseOffset<MultiIntegerFeatures>(MultiIntegerFeatures *features) {
    features->_offsets.push_back(features->_featureValues.size());
}

}
