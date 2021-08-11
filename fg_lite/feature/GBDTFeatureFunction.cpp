#include "fg_lite/feature/GBDTFeatureFunction.h"
#include "fg_lite/gbdt/GBDTPredictor.h"
#include "fg_lite/gbdt/FeatureMap.h"
#include "fg_lite/feature/FeatureFormatter.h"
#include "autil/StringUtil.h"
using namespace std;
using namespace autil;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, GBDTFeatureFunction);

const string GBDTFeatureFormatter::TREE_PREFIX_STR = "tree_";
const string GBDTFeatureFormatter::TREE_MIDDEL_STR = "_node_";

void GBDTFeatureFormatter::formatAndAddFeature(int tree, int node, MultiSparseFeatures *features) {
    char *buffer = _buffer + TREE_PREFIX_STR.size();
    buffer = addNumber(buffer, tree);
    memcpy(buffer, TREE_MIDDEL_STR.data(), TREE_MIDDEL_STR.size());
    buffer += TREE_MIDDEL_STR.size();
    buffer = addNumber(buffer, node);
    features->addFeatureKey(_buffer, buffer - _buffer);
}

char *GBDTFeatureFormatter::addNumber(char *buffer, int num) {
    if (num < 10) {
        *buffer++ = two_ASCII_digits[num][1];
        return buffer;
    } else if (num < 100) {
        *buffer++ = two_ASCII_digits[num][0];
        *buffer++ = two_ASCII_digits[num][1];
        return buffer;
    } else {
        string strNum = StringUtil::toString(num);
        return std::copy(strNum.begin(), strNum.end(), buffer);
    }
}

GBDTFeatureFunction::GBDTFeatureFunction(
        std::vector<FeatureFunction*> featureFunctions,
        GBDTPredictor *predictor)
    : FeatureFunction("GBDT")
    , _predictor(predictor)
{
    initFeatureFunctions(featureFunctions, _predictor->getFeatureMap());
}

GBDTFeatureFunction::~GBDTFeatureFunction() {
    for (auto &featureFunction : _featureFunctions) {
        delete featureFunction;
    }
    DELETE_AND_SET_NULL(_predictor);
}

void GBDTFeatureFunction::initFeatureFunctions(
        const std::vector<FeatureFunction*> &featureFuntions,
        const FeatureMap *featureMap)
{
    assert(_featureFunctions.empty());
    _featureFunctions.resize(featureMap->featureCount(), nullptr);
    for (auto &featureFunc : featureFuntions) {
        const string &featureName = featureFunc->getFeatureName();
        int32_t featureIdx = featureMap->getFeatureId(featureName);
        if (featureIdx >= 0) {
            if (featureIdx >= (int32_t)_featureFunctions.size()) {
                // featureMap feature size may be not match with featureFuntions
                _featureFunctions.resize(featureIdx + 1);
            }
            _featureFunctions[featureIdx] = featureFunc;
        } else {
            delete featureFunc;
        }
    }
}

inline float getInputValue(SingleDenseFeatures *feature, size_t id) {
    if (!feature) {
        return 0.0;
    }
    return feature->getFeatureValue(feature->count() == 1 ? 0 : id);
}

size_t GBDTFeatureFunction::getInputCount() const {
    size_t total = 0;
    for (size_t i = 0; i < _featureFunctions.size(); i++) {
        if (_featureFunctions[i] != nullptr) {
            total += _featureFunctions[i]->getInputCount();
        }
    }
    return total;
}

Features *GBDTFeatureFunction::genFeatures(
        const vector<FeatureInput*> &featureInputs,
        FeatureFunctionContext *context) const
{
    size_t docCount;
    if (!checkAndGetDocCount(featureInputs, docCount)) {
        AUTIL_LOG(ERROR, "feature[%s] docCounts not equal 1 or not one user input",
                  getFeatureName().c_str());
        return nullptr;
    }
    
    vector<SingleDenseFeatures*> features;
    features.reserve(_featureFunctions.size());
    size_t offset = 0;
    for (auto &featureFunc : _featureFunctions) {
        if (featureFunc == nullptr) {
            features.push_back(nullptr);
            continue;
        }
        size_t count = featureFunc->getInputCount();
        vector<FeatureInput*> subFeatureInputs(featureInputs.begin() + offset,
                featureInputs.begin() + offset + count);
        Features *genFeatures = featureFunc->genFeatures(subFeatureInputs, context);
        offset += count;
        if (genFeatures == nullptr) {
            // log failed to context, and return it to user.
            AUTIL_LOG(WARN, "compute feature[%s] for gbdt predictor failed!",
                      featureFunc->getFeatureName().c_str());
        }
        SingleDenseFeatures *singleFeatures = dynamic_cast<SingleDenseFeatures*>(genFeatures);
        if (genFeatures && !singleFeatures) {
            delete genFeatures;
            // do not continue
            // push NULL if feature compute failed.
            AUTIL_LOG(WARN, "gbdt predict only support single dense feature," 
                      "feature[%s] is not a single dense feature",
                      featureFunc->getFeatureName().c_str());
        }
        features.push_back(singleFeatures);
    }

    vector<float> inputs(features.size());
    GBDTPredictor::PredictDetailVec details;
    GBDTFeatureFormatter formatter;
    MultiSparseFeatures *outputFeatures = new MultiSparseFeatures(docCount);
    for (uint32_t i = 0; i < docCount; ++i) {
        inputs.assign(inputs.size(), 0.0);
        for (size_t j = 0; j < features.size(); ++j) {
            inputs[j] = getInputValue(features[j], i);
        }
        (void)_predictor->predict(inputs.data(), inputs.size(), details);
        outputFeatures->beginDocument();
        for (size_t j = 0; j < details.size(); j++) {
            formatter.formatAndAddFeature(details[j].first, details[j].second, outputFeatures);
        }
    }
    for (auto &feature : features) {
        delete feature;
    }
    return outputFeatures;
}

}
