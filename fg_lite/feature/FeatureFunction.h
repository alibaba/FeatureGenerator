#ifndef ISEARCH_FG_LITE_FEATUREFUNCTION_H
#define ISEARCH_FG_LITE_FEATUREFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/Feature.h"
#include "fg_lite/feature/FeatureInput.h"
#include "fg_lite/feature/FeatureFormatter.h"

namespace fg_lite {

#define FEATURE_SEPARATOR '_'

struct FeatureFunctionContext {
public:
    // index partition reader.
};

class FeatureFunction
{
public:

    enum OutputValueType{
        OVT_INTEGER,
        OVT_STRING
    };

    FeatureFunction(const std::string &featureName = "",
                    const std::string &featurePrefix = "");
    virtual ~FeatureFunction() = default;
private:
    FeatureFunction(const FeatureFunction &);
    FeatureFunction& operator=(const FeatureFunction &);
public:
    virtual Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                                  FeatureFunctionContext *context) const = 0;
    virtual size_t getInputCount() const = 0;
public:
    const std::string &getFeatureName() const { return _featureName; }
    static Features *maybeDefaultBucketize(const std::string &name, const std::vector<float> &boundaries, int count);
protected:
    FeatureFormatter::FeatureBuffer getFeaturePrefix(autil::mem_pool::PoolBase *pool) const {
        return FeatureFormatter::FeatureBuffer(_featurePrefix.begin(), _featurePrefix.end(),
                autil::mem_pool::pool_allocator<char>(pool));
    }
    bool checkAndGetDocCount(const std::vector<FeatureInput*> &inputs,
                             size_t &docCount) const;
    bool checkInput(const std::vector<FeatureInput*> &inputs) const;
private:
    std::string _featureName;
    std::string _featurePrefix;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_FEATUREFUNCTION_H
