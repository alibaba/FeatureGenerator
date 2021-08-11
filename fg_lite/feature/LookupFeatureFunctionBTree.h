#pragma once

#include "autil/MurmurHash.h"

#include "fg_lite/feature/LookupFeatureBTreeClient.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

namespace fg_lite {

class LookupFeatureFunctionBTree : public FeatureFunction
{
public:
    LookupFeatureFunctionBTree(
            const std::string &name,
            const Normalizer &normalizer,
            const std::string &combiner,
            uint32_t dimension = 1,
            const std::vector<float> &boundaries = std::vector<float>());

private:
    LookupFeatureFunctionBTree(const LookupFeatureFunctionBTree &);
    LookupFeatureFunctionBTree& operator=(const LookupFeatureFunctionBTree &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context)const override;

    size_t getInputCount() const override {
        return 2; // item & user
    }
private:
    template<CombinerType type, typename FeatureT = MultiDenseFeatures>
    void generate(const std::vector<uint64_t> *keys,
                  const autil::MultiChar &value,
                  FeatureT *features) const;

    template<LookupFeatureV3KeyType KeyHashType,
             LookupFeatureV3ValueType ValueEncodeType,
             typename Collector>
    void doBranchFind(const autil::MultiChar& buffer,
                      Collector &collector,
                      size_t dimension) const{
        using KeyType = typename MatchHashType<KeyHashType>::HashT;
        using ValueType = typename MatchValueType<ValueEncodeType>::ValueT;
        const auto isNotFound = MatchValueType<ValueEncodeType>::isNotFound;
        const auto finalHashFunction = MatchHashType<KeyHashType>::F;
        while (!collector.end()) {
            KeyType key = finalHashFunction(collector.getKey());
            auto pos = (ValueType*) branchFind<KeyType>(autil::ConstString(buffer.data(), buffer.size()), key, sizeof(ValueType), _dimension);
            if (pos) {
                for (size_t fieldIndex = 0; fieldIndex < _dimension; ++fieldIndex) {
                    // collect float type anyway
                    ValueType value = pos[fieldIndex];
                    if (!isNotFound(value)) {
                        collector.collect((float)value, fieldIndex);
                    }
                }
            }
        }
    }

private:
    template<typename KeyType>
    std::vector<KeyType> genKeyFromUserInput(FeatureInput *userInput) const {
        std::vector<KeyType> keys;
        auto typedInput = dynamic_cast<FeatureInputTyped<std::string, DenseStorage<std::string>>*> (userInput);
        if (typedInput) {
            for (size_t i = 0; i < typedInput->col(0); i++) {
                auto value = typedInput->get(0, i);
                keys.push_back(autil::MurmurHash::MurmurHash64A(value.data(), value.size(), 0));
            }
        }
        return keys;
    }

private:
    Normalizer _normalizer;
    CombinerType _combinerType;
    uint32_t _dimension;
    std::vector<float> _boundaries;
    LookupFeatureV3KeyType _keyType;
    LookupFeatureV3ValueType _valueType;
private:
    AUTIL_LOG_DECLARE();
};

} // namespace fg_lite
