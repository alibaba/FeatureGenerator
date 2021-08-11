#include "fg_lite/feature/OverLapFeatureFunction.h"

using namespace std;
using namespace autil;

namespace fg_lite {

AUTIL_LOG_SETUP(fg_lite, OverLapFeatureFunction);

OverLapFeatureFunction::OverLapFeatureFunction(
        const string &featureName,
        OverLapFeatureConfig::OverLapType type,
        const string &separator,
        bool needDiscrete,
        bool needPrefix,
        bool needDense,
        int cutThreshold)
    : FeatureFunction(featureName)
    , _seperator(separator)
    , _type(type)
    , _needDiscrete(needDiscrete)
    , _needPrefix(needPrefix)
    , _needDense(needDense)
    , _cutThreshold(cutThreshold)
{
}

Features *OverLapFeatureFunction::genFeatures(
        const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "input size is %lu", inputs.size());
        return nullptr;
    }
    FeatureInput *userInput = inputs[0];
    FeatureInput *itemInput = inputs[1];

    if (userInput->row() == 0) {
        AUTIL_LOG(WARN, "feature[%s]: no user input", getFeatureName().c_str());
        return nullptr;
    }

    return genNoCombo(userInput, itemInput);
}

}
