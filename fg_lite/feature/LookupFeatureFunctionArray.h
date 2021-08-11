#ifndef ISEARCH_FG_LITE_LOOKUPFEATUREFUNCTIONARRAY_H
#define ISEARCH_FG_LITE_LOOKUPFEATUREFUNCTIONARRAY_H

#include "autil/Log.h"
#include "autil/StringUtil.h"
#include "fg_lite/feature/Combiner.h"
#include "fg_lite/feature/FeatureFunction.h"

using namespace autil;

namespace fg_lite {

class LookupFeatureFunctionArray : public FeatureFunction
{
public:
   LookupFeatureFunctionArray(const std::string &name,
                              const std::string &prefix,
                              const std::string &defaultLookupResult,
                              bool hasDefault,
                              bool needDiscrete,
                              const std::vector<float> &boundaries,
                              const std::string &combiner = "sum",
                              float timeDiff = -1.0f,
                              const std::string &combiner2 = "none",
                              bool needCombo = false,
                              int count1CutThreshold = -1,
                              int count2CutThreshold = -1,
                              bool comboRight = true,
                              bool comboSimple = false);
    ~LookupFeatureFunctionArray() = default;
private:
    LookupFeatureFunctionArray(const LookupFeatureFunctionArray &);
    LookupFeatureFunctionArray& operator=(const LookupFeatureFunctionArray &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        if (_timeDiff >= 0) {
            if (_needCombo) {
                if (_comboSimple) {
                    return 6;
                }
                return 8;
            } else {
                return 5;
            }
        }
        return 3;
    }
private:
    // key and mapKey must has the same Type!
    // key and other params may have different StorageType!
    template<typename KeyType, typename MapKeyType, template<typename> class KStorageType, template<typename> class StorageType, typename MapValueType>
    Features* lookupAllTyped(
            const FeatureInputTyped<KeyType, KStorageType<KeyType>> *key,
            const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *mapKey,
            const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *mapValue,
            const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *pvtime = nullptr,
            const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *urbtimes = nullptr,
            const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *map2Key = nullptr,
            const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *map2Value = nullptr,
            const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *urbtimes2 = nullptr,
            const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *otherValue = nullptr) const;

    template <typename KeyType, typename MapKeyType, template <typename> class KStorageType,
              template <typename> class StorageType, typename MapValueType, typename FeaturesType>
    void fillFeatureValue(
        const FeatureInputTyped<KeyType, KStorageType<KeyType>> *key,
        const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *mapKey,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *mapValue,
        FeaturesType *features) const;

 private:
    const std::string _defaultLookupResult;
    const bool _hasDefault;
    const bool _needDiscrete;
    const float _timeDiff;
    const CombinerType _combinerType;
    const CombinerType _combiner2Type;
    const bool _needCombo;
    int _count1CutThreshold;
    int _count2CutThreshold;
    bool _comboRight;
    bool _comboSimple;
    const std::vector<float> _boundaries;
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_LOOKUPFEATUREFUNCTIONARRAY_H
