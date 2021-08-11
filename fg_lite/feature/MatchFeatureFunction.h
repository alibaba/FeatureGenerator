#ifndef ISEARCH_FG_LITE_MATCHFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_MATCHFEATUREFUNCTION_H

#include "autil/Log.h"
#include "autil/MultiValueType.h"
#include "autil/ConstString.h"

#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/UserMatchInfo.h"
#include "fg_lite/feature/MatchFunction.h"

namespace fg_lite {

class UserIterator;

class MatchFeatureFunction : public FeatureFunction
{
public:
    MatchFeatureFunction(const MatchFunction *matcher,
                         bool wildCardCategory,
                         bool wildCardItem);
private:
    MatchFeatureFunction(const MatchFeatureFunction &);
    MatchFeatureFunction& operator=(const MatchFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return _wildCardCategory || _wildCardItem ? 2 : 3;
    }
private:
    template <typename FeatureType>
    void genMatchFeatures(FeatureInput *itemInput,
                          FeatureInput *categoryInput,
                          UserIterator &userIterator,
                          FeatureType *features) const;
private:
    std::unique_ptr<const MatchFunction> _matcher;
    bool _wildCardCategory;
    bool _wildCardItem;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_MATCHFEATUREFUNCTION_H
