#pragma once

#include "autil/Log.h"
#include "autil/MurmurHash.h"

#include "fg_lite/feature/Collector.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"
#include "fg_lite/feature/LookupFeatureSparseEncoder.h"


namespace fg_lite {

// keyExpr should be user feature
class LookupFeatureFunctionV2 : public FeatureFunction
{
public:
    LookupFeatureFunctionV2(const std::string &name,
                            const Normalizer &normalizer,
                            const std::string &combiner,
                            uint32_t dimension,
                            const std::vector<float> &boundaries,
                            bool useHeader=false,
                            bool useSparse=false,
                            LookupFeatureV3KeyType keyType = LOOKUP_V3_KEY_HASH_0_TO_63_BIT,
                            LookupFeatureV3ValueType valueType = LOOKUP_V3_VALUE_ENCODE_32BIT
                            );
private:
    LookupFeatureFunctionV2(const LookupFeatureFunctionV2 &);
    LookupFeatureFunctionV2& operator=(const LookupFeatureFunctionV2 &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return 2;
    }
private:
    std::vector<uint64_t> genKey(FeatureInput *userInput) const;
    template<CombinerType type, typename FeatureT>
    void generate(const std::vector<uint64_t> *keys,
                  FeatureInputTyped<autil::MultiChar, DenseStorage<autil::MultiChar>> *input,
                  FeatureT *features) const;
    template<CombinerType type>
    void generateMultiDense(const std::vector<uint64_t> *keys,
                            FeatureInputTyped<autil::MultiChar, DenseStorage<autil::MultiChar>> *input,
                            MultiDenseFeatures *features, uint32_t multiValueDimension) const;
    template<typename Collector>
    static void matchWithHeader(const autil::MultiChar &data,
                      Collector &collector,
                      uint32_t dim=1);

    // use _keyType, _valueType, non-static
    template<typename Collector>
    void matchSparseEncode(
            const autil::MultiChar &data, Collector &collector, uint32_t dim) const;

    template<typename Collector>
    void match(const char *data, size_t len, Collector &collector) const;


private:
    Normalizer _normalizer;
    CombinerType _combinerType;
    uint32_t _dimension;
    std::vector<float> _boundaries;
    bool _useHeader;
    bool _useSparse;
    LookupFeatureV3KeyType _keyType;
    LookupFeatureV3ValueType _valueType;
private:
    AUTIL_LOG_DECLARE();
};

}
