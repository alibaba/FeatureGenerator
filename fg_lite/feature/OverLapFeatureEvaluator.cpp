#include "fg_lite/feature/OverLapFeatureEvaluator.h"

using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, OverLapFeatureEvaluator);

OverLapFeatureEvaluator::OverLapFeatureEvaluator(
        const string &featureName,
        const string &separator,
        const OverLapFeatureConfig::OverLapType &type,
        int cutThreshold)
    : _separator(separator)
    , _pool(1024)
    , _buffer(cp_alloc(&_pool))
    , _type(type)
    , _cutThreshold(cutThreshold)
{
    _buffer.assign(featureName.begin(), featureName.end());
    _buffer.insert(_buffer.end(), separator.begin(), separator.end());
}

}
