#include "fg_lite/feature/MatchFunction.h"
#include "fg_lite/feature/MatchFunctionImpl.h"

using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, MatchFunction);

MatchFunction::MatchFunction(const string &featureNamePrefix, bool needDiscrete,
                             bool showCategory, bool showItem, bool needWeighting)
    : _featureNamePrefix(featureNamePrefix)
    , _needDiscrete(needDiscrete)
    , _showCategory(showCategory)
    , _showItem(showItem)
    , _needWeighting(needWeighting)
{
}

MatchFunction *MatchFunction::create(const string &matchType,
                                     const string &featureNamePrefix,
                                     const string &normalizerDesc,
                                     bool needDiscrete,
                                     bool showCategory,
                                     bool showItem,
                                     bool needWeighting)
{
    if (!normalizerDesc.empty()) {
        if (matchType != "hit") {
            AUTIL_LOG(ERROR, "only hit support normalize.");
            return NULL;
        }
        if (needDiscrete) {
            AUTIL_LOG(ERROR, "needDiscrete does not need normalize");
            return NULL;
        }
    }
    Normalizer normalizer;
    if (!normalizer.parse(normalizerDesc)) {
        AUTIL_LOG(ERROR, "create normalizer failed. normalizer[%s]",
                  normalizerDesc.c_str());
        return NULL;
    }

    MatchFunction *matcher = NULL;
    if (matchType == "hit") {
        matcher = new HitMatch(featureNamePrefix, needDiscrete, needWeighting);
    } else if (matchType == "multihit") {
        matcher = new MultiHitMatch(featureNamePrefix, needDiscrete,
                showCategory, showItem, needWeighting);
    } else if (matchType == "cross") {
        if (!needDiscrete) {
            AUTIL_LOG(ERROR, "cross match only support discrete");
        } else {
            matcher = new CrossMatch(featureNamePrefix);
        }
    } else if (matchType == "cos") {
        matcher = new CosMatch(featureNamePrefix, needDiscrete);
    } else {
        AUTIL_LOG(ERROR, "invalid match type:[%s]", matchType.c_str());
    }
    if (matcher) {
        matcher->setNormalizer(normalizer);
    }
    return matcher;
}

}
