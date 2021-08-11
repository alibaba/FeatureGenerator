#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/Normalizer.h"
#include <set>

using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, FeatureFunction);

FeatureFunction::FeatureFunction(const string &featureName,
                                 const string &featurePrefix)
    : _featureName(featureName)
    , _featurePrefix(featurePrefix)
{
}

bool FeatureFunction::checkAndGetDocCount(
        const vector<FeatureInput*> &inputs,
        size_t &docCount) const
{
    set<size_t> docCounts;
    for (size_t i = 0; i < inputs.size(); i++) {
        docCounts.insert(inputs[i]->row());
    }
    bool valid = docCounts.size() == 1
                 || (docCounts.size() == 2 && *(docCounts.begin()) <= 1);
    if (!valid) {
        return false;
    }
    docCount = *(docCounts.rbegin());
    return true;
}

bool FeatureFunction::checkInput(const vector<FeatureInput*> &inputs) const {
    return inputs.size() == getInputCount();
}

Features *FeatureFunction::maybeDefaultBucketize(
        const string &name, const std::vector<float> &boundaries, int count)
{
    if (!boundaries.empty()) {
        auto newfeatures = new SingleIntegerFeatures(name, count);
        auto value = bucketize(0, boundaries);
        for (int i = 0; i < count; i++) {
            newfeatures->addFeatureValue(value);
        }
        return newfeatures;
    }
    return nullptr;
}

}
