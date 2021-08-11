#ifndef ISEARCH_FG_LITE_RAWFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_RAWFEATUREFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/Normalizer.h"

namespace fg_lite {

class RawFeatureFunction : public FeatureFunction
{
public:
    RawFeatureFunction(const std::string &featureName,
                       const Normalizer &normalizer,
                       const std::vector<float> &boundaries,
                       int valueDimension)
        : FeatureFunction(featureName)
        , _normalizer(normalizer)
        , _boundaries(boundaries)
        , _valueDimension(valueDimension)
    {}
private:
    RawFeatureFunction(const RawFeatureFunction &);
    RawFeatureFunction& operator=(const RawFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return 1;
    }
private:
    template<typename FeatureType>
    Features *genDenseFeatures(FeatureInput *input) const;
    template<typename FeatureType>
    Features *genSparseFeatures(FeatureInput *input) const;
    template <typename T, typename FeatureType, typename StorageType>
    void genFeaturesTyped(FeatureInput *input, FeatureType *features) const;
    void addFeature(SingleDenseFeatures *features, float value) const;
    void addFeature(SingleIntegerFeatures *features, float value) const;
    template<typename Features>
    static void appendSparseOffset(Features *features);
private:
    Normalizer _normalizer;
    std::vector<float> _boundaries;
    int _valueDimension;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_RAWFEATUREFUNCTION_H
