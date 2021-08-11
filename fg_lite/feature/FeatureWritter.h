#ifndef ISEARCH_FG_LITE_FEATUREWRITTER_H
#define ISEARCH_FG_LITE_FEATUREWRITTER_H

#include "autil/Log.h"
#include "autil/ConstString.h"
#include "fg_lite/feature/FeatureFunction.h"

namespace fg_lite {

class SparseFeatureWritter {
public:
    typedef MultiSparseFeatures FeaturesType;
public:
    SparseFeatureWritter(FeaturesType *f)
        :features(f)
    {
        features->beginDocument();
    }
    ~SparseFeatureWritter() = default;
private:
    SparseFeatureWritter(const SparseFeatureWritter &);
    SparseFeatureWritter& operator=(const SparseFeatureWritter &);
public:
    virtual void addFeature(const ConstString &key, const ConstString &value,
                            FeatureFormatter::FeatureBuffer &buffer) = 0;
public:
    FeaturesType *features;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_FEATUREWRITTER_H
