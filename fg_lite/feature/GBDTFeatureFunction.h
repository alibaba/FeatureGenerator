#ifndef ISEARCH_FG_LITE_GBDTFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_GBDTFEATUREFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"

namespace fg_lite {

class GBDTPredictor;
class FeatureGenerator;
class FeatureMap;

class GBDTFeatureFormatter {
public:
    static const std::string TREE_PREFIX_STR;
    static const std::string TREE_MIDDEL_STR;
public:
    GBDTFeatureFormatter() {
        memcpy(_buffer, TREE_PREFIX_STR.data(), TREE_PREFIX_STR.size());
    }
public:
    void formatAndAddFeature(int tree, int node, MultiSparseFeatures *features);
private:
    char *addNumber(char *buffer, int num);
private:
    char _buffer[64];
};

class GBDTFeatureFunction : public FeatureFunction
{
public:
    GBDTFeatureFunction(std::vector<FeatureFunction*> featureFunctions,
                        GBDTPredictor *predictor);
    ~GBDTFeatureFunction();
private:
    GBDTFeatureFunction(const GBDTFeatureFunction &);
    GBDTFeatureFunction& operator=(const GBDTFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &featureInputs,
                          FeatureFunctionContext *context) const override;
protected:
    size_t getInputCount() const override;
private:
    void initFeatureFunctions(
            const std::vector<FeatureFunction*> &featureFuntions,
            const FeatureMap *featureMap);

private:
    std::vector<FeatureFunction*> _featureFunctions;
    GBDTPredictor *_predictor;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_GBDTFEATUREFUNCTION_H
