#pragma once

#include "autil/Log.h"
#include "fg_lite/feature/Combiner.h"
#include "fg_lite/feature/Collector.h"
#include "fg_lite/feature/Normalizer.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

namespace fg_lite {

class LookupFeatureFunctionV3 : public FeatureFunction
{
public:
    LookupFeatureFunctionV3(const std::string &name,
                            const Normalizer &normalizer,
                            const std::string &combiner,
                            uint32_t dimension,
                            const std::vector<float> &boundaries);
private:
    LookupFeatureFunctionV3(const LookupFeatureFunctionV3 &);
    LookupFeatureFunctionV3& operator=(const LookupFeatureFunctionV3 &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return 2;
    }
private:
    std::vector<uint64_t> genKey(const FeatureInput *userInput) const;
    template<CombinerType type, typename FeatureT = MultiDenseFeatures>
    void generate(const std::vector<uint64_t> *keys,
                  const autil::MultiChar &value,
                  FeatureT *features) const;
    template<typename Collector>
    void match(const autil::MultiChar &data, const uint32_t dim, Collector &collector) const;

private:
    Normalizer _normalizer;
    CombinerType _combinerType;
    uint32_t _dimension;
    std::vector<float> _boundaries;

private:
    AUTIL_LOG_DECLARE();
};

}
