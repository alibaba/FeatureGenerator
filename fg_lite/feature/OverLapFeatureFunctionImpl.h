#include "fg_lite/feature/OverLapFeatureFunction.h"
#include "fg_lite/feature/OverLapFeatureEvaluator.h"

using namespace std;
using namespace autil;

namespace fg_lite {

template <InputDataType userDT, InputStorageType userST,InputDataType itemDT, InputStorageType itemST>
Features* OverLapFeatureFunction::genFeaturesTyped(
        FeatureInput *user, FeatureInput *item) const
{
    typedef typename DTST2InputType<userDT, userST>::InputType UserInputType;
    typedef typename DTST2InputType<itemDT, itemST>::InputType ItemInputType;

    UserInputType *userInput = dynamic_cast<UserInputType*>(user);
    if (userInput == nullptr) {
        AUTIL_LOG(ERROR, "feature[%s]: user input dynamic_cast error",
                  getFeatureName().c_str());
        return nullptr;
    }
    ItemInputType *itemInput = dynamic_cast<ItemInputType*>(item);
    if (itemInput == nullptr) {
        AUTIL_LOG(ERROR, "feature[%s]: item input dynamic_cast error",
                  getFeatureName().c_str());
        return nullptr;
    }

    if(userInput->row() > 1 && userInput->row() != itemInput->row()){
        AUTIL_LOG(ERROR, "feature[%s]: user and item row size not consistent",
                  getFeatureName().c_str());
        return nullptr;
    }

    if (_needDiscrete && !_needDense) {      // needDiscrete is true and needDense is false
        unique_ptr<MultiSparseFeatures> features(new MultiSparseFeatures(itemInput->row()));
        genFeaturesTypedInner(userInput, itemInput, features.get());
        return features.release();
    } else if (_needDense && _needPrefix) {  // needDiscrete is true and needDense is true and needPrefix true
        unique_ptr<SingleSparseFeatures> features(new SingleSparseFeatures(itemInput->row()));
        genFeaturesTypedInnerSingleSparse(userInput, itemInput, features.get());
        return features.release();
    } else { // needDiscrete is false
        unique_ptr<SingleDenseFeatures> features(new SingleDenseFeatures(getFeatureName(), itemInput->row()));
        genFeaturesTypedInner(userInput, itemInput, features.get());
        return features.release();
    }
}

template <typename InputType1, typename InputType2, typename FeatureType>
void OverLapFeatureFunction::genFeaturesTypedInner(
        InputType1 *userInput, InputType2 *itemInput, FeatureType *features) const
{
    vector<ConstString> userValues;
    vector<ConstString> itemValues;
    bool userMultiRow = userInput->row() > 1;
    unique_ptr<OverLapFeatureEvaluator> evaluator(new OverLapFeatureEvaluator(
                    getFeatureName(), _seperator, _type, _cutThreshold));
    for (size_t i = 0; i < itemInput->row(); i++) {
        features->beginDocument();
        itemValues.clear();
        userValues.clear();
        size_t userRowI = 0;
        if(userMultiRow) {
            userRowI = i;
        }
        auto userRow = getRowData(userInput, userRowI, userValues);
        auto itemRow = getRowData(itemInput, i, itemValues);
        evaluator->genFeature(userRow, itemRow, features, static_cast<decltype(itemRow)*>(nullptr));
    }
}

template <typename InputType1, typename InputType2>
void OverLapFeatureFunction::genFeaturesTypedInnerSingleSparse(InputType1 *userInput, InputType2 *itemInput, SingleSparseFeatures *features) const {
    vector<ConstString> userValues;
    vector<ConstString> itemValues;
    bool userMultiRow = userInput->row() > 1;
    unique_ptr<OverLapFeatureEvaluator> evaluator(new OverLapFeatureEvaluator(
                    getFeatureName(), _seperator, _type, _cutThreshold));
    for (size_t i = 0; i < itemInput->row(); i++) {
        itemValues.clear();
        userValues.clear();
        size_t userRowI = 0;
        if(userMultiRow) {
            userRowI = i;
        }
        auto userRow = getRowData(userInput, userRowI, userValues);
        auto itemRow = getRowData(itemInput, i, itemValues);
        evaluator->genFeature(userRow, itemRow, features, static_cast<decltype(itemRow)*>(nullptr));
    }
}

#define OVERLAP_IMPL_2(userDT, userST, itemDT, itemST) \
    template Features* OverLapFeatureFunction::genFeaturesTyped\
        <userDT, userST, itemDT, itemST>(FeatureInput *user, FeatureInput *item) const;
}
