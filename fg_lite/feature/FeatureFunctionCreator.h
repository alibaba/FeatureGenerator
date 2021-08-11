#ifndef ISEARCH_FEATURE_FUNCTION_CREATOR_H
#define ISEARCH_FEATURE_FUNCTION_CREATOR_H

namespace fg_lite {
class FeatureFunction;
class FeatureConfig;
class SingleFeatureConfig;

class FeatureFunctionCreator
{
private:
    FeatureFunctionCreator();
    ~FeatureFunctionCreator() = default;
public:
    static FeatureFunction *createFeatureFunction(
            const SingleFeatureConfig *featureConfig);
};
}

#endif //ISEARCH_FEATURE_FUNCTION_CREATOR_H
