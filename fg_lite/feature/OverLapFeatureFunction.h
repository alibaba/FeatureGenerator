#ifndef ISEARCH_FG_LITE_OVERLAPFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_OVERLAPFEATUREFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/AnyConvert.h"
#include "fg_lite/feature/FeatureConfig.h"

namespace fg_lite {

class OverLapFeatureFunction : public FeatureFunction
{
public:
    OverLapFeatureFunction(const std::string &featureName,
                           OverLapFeatureConfig::OverLapType type,
                           const std::string &separator = "_",
                           bool needDiscrete = true,
                           bool needPrefix = false,
                           bool needDense = false,
                           int cutThreshold = -1);
private:
    OverLapFeatureFunction(const OverLapFeatureFunction &);
    OverLapFeatureFunction& operator=(const OverLapFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return 2;
    }


private:
    template <InputDataType userDT, InputStorageType userST,
              InputDataType itemDT, InputStorageType itemST>
    Features* genFeaturesTyped(FeatureInput *user, FeatureInput *item) const;

    template <typename InputType1, typename InputType2, typename FeatureType>
    void genFeaturesTypedInner(InputType1 *userInput, InputType2 *itemInput, FeatureType *features) const;

    template <typename InputType1, typename InputType2>
    void genFeaturesTypedInnerSingleSparse(InputType1 *userInput, InputType2 *itemInput, SingleSparseFeatures *features) const;

    template<typename DataType, typename StorageType>
    Row<typename std::enable_if<IsStringType<DataType>::value, autil::ConstString>::type>
    getRowData(FeatureInputTyped<DataType, StorageType> *input, size_t i,
               std::vector<autil::ConstString> &values) const
    {
        values.clear();
        for(size_t j = 0; j < input->col(i); ++j){
            const DataType value = input->get(i, j);
            values.push_back(autil::ConstString(value.data(), value.size()));
        }
        Row<autil::ConstString> inputRow(values.data(), values.size());
        return inputRow;
    }

    template<typename DataType, typename StorageType >
    Row<typename std::enable_if<!IsStringType<DataType>::value, DataType>::type>
    getRowData(FeatureInputTyped<DataType, StorageType> *input, size_t i,
               std::vector<autil::ConstString> &values) const
    {
        return input->getRow(i);
    }

    Features* genNoCombo(FeatureInput *userInput, FeatureInput *itemInput) const;
private:
    std::string _seperator;
    OverLapFeatureConfig::OverLapType _type;
    bool _needDiscrete;
    bool _needPrefix;
    bool _needDense;
    int _cutThreshold;
private:
    AUTIL_LOG_DECLARE();
};

#define DEFAULT(message, feature, type)         \
    default:                                    \
    break;


#define CASE2(userDT, userST, itemDT, itemST)                           \
    case userST : {                                                     \
        return genFeaturesTyped<userDT, userST, itemDT, itemST>(userInput, itemInput); \
    }                                                                   \
        break;

#define CASE_SWITCH_USER_ST2(itemDT, userDT, itemST)                    \
    case itemST: {                                                      \
        switch (userStorageType) {                                      \
            CASE2(userDT, IST_DENSE, itemDT, itemST);                   \
            CASE2(userDT, IST_SPARSE_MULTI_VALUE, itemDT, itemST);      \
            DEFAULT(m1, featureName, userStorageType);                  \
        }                                                               \
    }                                                                   \
    break;

#define CASE_SWITCH_ITEM_ST2(itemDT, userDT)                            \
    case userDT: {                                                      \
        switch (itemStorageType) {                                      \
            CASE_SWITCH_USER_ST2(itemDT, userDT, IST_DENSE);            \
            CASE_SWITCH_USER_ST2(itemDT, userDT, IST_SPARSE_MULTI_VALUE); \
            DEFAULT(m1, featureName, itemStorageType);                  \
        }                                                               \
    }                                                                   \
    break;

#define CASE_SWITCH_USER_DATATYPE2(itemDT)                              \
    case itemDT: {                                                      \
        switch (userDataType) {                                         \
            TWO_INPUT_DATA_TYPE_MACRO_HELPER(itemDT, CASE_SWITCH_ITEM_ST2); \
            DEFAULT(m1, featureName, userDataType);                     \
        }                                                               \
    }                                                                   \
    break;                                                              \

}

#endif //ISEARCH_FG_LITE_OVERLAPFEATUREFUNCTION_H
