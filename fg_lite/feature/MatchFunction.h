#ifndef ISEARCH_FG_LITE_MATCHFUNCTION_H
#define ISEARCH_FG_LITE_MATCHFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/UserMatchInfo.h"
#include "fg_lite/feature/Feature.h"
#include "fg_lite/feature/Normalizer.h"

namespace fg_lite {

class MatchFunction
{
public:
    MatchFunction(const std::string &featureNamePrefix, bool needDiscrete = true,
                  bool showCategory = true, bool showItem = true, bool needWeighting = false);
    virtual ~MatchFunction() = default;
private:
    MatchFunction(const MatchFunction &);
    MatchFunction& operator=(const MatchFunction &);
public:
    static MatchFunction *create(const std::string &matchType,
                                 const std::string &featureNamePrefix,
                                 const std::string &normalizerDesc,
                                 bool needDiscrete, bool showCategory,
                                 bool showItem, bool needWeighting = false);
public:
    virtual void matchOneFeature(const autil::ConstString &category,
                                 const autil::ConstString &itemValue,
                                 const UserMatchInfo &userMatchInfo,
                                 MultiDenseFeatures *feature) const = 0;
    virtual void matchOneFeature(const autil::ConstString &category,
                                 const autil::ConstString &itemValue,
                                 const UserMatchInfo &userMatchInfo,
                                 MultiSparseFeatures *feature) const = 0;
    virtual void matchOneFeature(const autil::ConstString &category,
                                 const autil::ConstString &itemValue,
                                 const UserMatchInfo &userMatchInfo,
                                 MultiSparseWeightingFeatures *features) const = 0;
    bool isSparseFeature() const { return _needDiscrete; }

    bool needWeighting() const { return _needWeighting; }

    const std::string &getFeatureName() const { return _featureNamePrefix; }

private:
    // only Match support, do check in MatchFunction::create
    void setNormalizer(const Normalizer &normalizer) {
        _normalizer = normalizer;
    }
protected:
    std::string _featureNamePrefix;
    bool _needDiscrete;
    bool _showCategory;
    bool _showItem;
    bool _needWeighting;
    Normalizer _normalizer;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_MATCHFUNCTION_H
