#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunctionCreator.h"
#include "fg_lite/feature/FeatureConfig.h"
#include "fg_lite/feature/IdFeatureFunction.h"
#include "fg_lite/feature/RawFeatureFunction.h"
#include "fg_lite/feature/ComboFeatureFunction.h"
#include "fg_lite/feature/LookupFeatureFunction.h"
#include "fg_lite/feature/LookupFeatureFunctionV2.h"
#include "fg_lite/feature/LookupFeatureFunctionV3.h"
#include "fg_lite/feature/LookupFeatureFunctionArray.h"
#include "fg_lite/feature/MatchFeatureFunction.h"
#include "fg_lite/feature/MatchFunction.h"
#include "fg_lite/feature/GBDTFeatureFunction.h"
#include "fg_lite/feature/OverLapFeatureFunction.h"
#include "fg_lite/feature/KgbMatchSemanticFeatureFunction.h"
#include "fg_lite/feature/PreclickUrbWordFeatureFunction.h"

using namespace std;
using namespace autil;

namespace fg_lite {

AUTIL_DECLARE_AND_SETUP_LOGGER(fg_lite, FeatureFunctionCreator);

static FeatureFunction *createRawFeatureFunction(const RawFeatureConfig &featureConfig) {
    Normalizer normalizer;
    if (!normalizer.parse(featureConfig.normalizer)) {
        AUTIL_LOG(ERROR, "create normalizer failed. config[%s]",
                  ToJsonString(featureConfig).c_str());
        return nullptr;
    }
    return new RawFeatureFunction(featureConfig.getFeatureName(), normalizer,
                                  featureConfig.getBoundaries(),
                                  featureConfig.valueDimension);
}

FeatureFunction *createMatchFeatureFunction(const MatchFeatureConfig &featureConfig) {
    bool wildCardCategory = featureConfig.categoryExpression == MATCH_WILDCARD_STRING;
    bool wildCardItem = featureConfig.itemExpression == MATCH_WILDCARD_STRING;

    MatchFunction *matcher = MatchFunction::create(
            featureConfig.matchType,
            featureConfig.getFeatureName(),
            featureConfig.normalizer,
            featureConfig.needDiscrete,
            featureConfig.showCategory,
            featureConfig.showItem,
            featureConfig.needWeighting);
    if (matcher == nullptr) {
        AUTIL_LOG(ERROR, "create match function[%s] failed",
                  featureConfig.matchType.c_str());
        return nullptr;
    }
    return new MatchFeatureFunction(matcher, wildCardCategory, wildCardItem);
}

enum class LookupFeatureFunctionVersion {
    V1, V2, V3, BTree
};

FeatureFunction *createLookupFeatureFunction(
        const LookupFeatureConfig &featureConfig, LookupFeatureFunctionVersion version)
{
    if (version >= LookupFeatureFunctionVersion::V2 && featureConfig.needDiscrete) {
        AUTIL_LOG(ERROR, "LookupFeatureConfigV2/V3 does not support discrete");
        return nullptr;
    }

    Normalizer normalizer;
    if (!normalizer.parse(featureConfig.normalizer)) {
        AUTIL_LOG(ERROR, "create normalizer failed. config[%s]",
                  ToJsonString(featureConfig).c_str());
        return nullptr;
    }

    if (!featureConfig.mapKeysExpression.empty() &&
        !featureConfig.mapValuesExpression.empty())
    {
        if (featureConfig.timediff >= 0.0f) {
            return new LookupFeatureFunctionArray(
                featureConfig.getFeatureName(),
                featureConfig.getFeaturePrefix(),
                featureConfig.defaultLookupValue,
                featureConfig.hasDefault,
                featureConfig.needDiscrete,
                featureConfig.getBoundaries(),
                featureConfig.combiner,
                featureConfig.timediff,
                featureConfig.combiner2,
                featureConfig.needCombo,
                featureConfig.countCutThreshold,
                featureConfig.count2CutThreshold,
                featureConfig.comboRight,
                featureConfig.comboSimple);
        }
        return new LookupFeatureFunctionArray(
                featureConfig.getFeatureName(),
                featureConfig.getFeaturePrefix(),
                featureConfig.defaultLookupValue,
                featureConfig.hasDefault,
                featureConfig.needDiscrete,
                featureConfig.getBoundaries());
    }
    else if (LookupFeatureFunctionVersion::V1 == version) {
        return new LookupFeatureFunction(
                featureConfig.getFeatureName(),
                featureConfig.getFeaturePrefix(),
                featureConfig.needDiscrete,
                featureConfig.needKey,
                normalizer,
                featureConfig.combiner,
                featureConfig.valueDimension,
                featureConfig.needWeighting,
                featureConfig.isOptimized,
                featureConfig.getBoundaries(),
                featureConfig.defaultLookupValue,
                featureConfig.hasDefault);
    } else if (LookupFeatureFunctionVersion::V2 == version) {
        const auto &lookupFeatureConfigV2 = dynamic_cast<const LookupFeatureConfigV2&>(featureConfig);
        return new LookupFeatureFunctionV2(
                featureConfig.getFeatureName(),
                normalizer,
                featureConfig.combiner,
                featureConfig.valueDimension,
                featureConfig.getBoundaries(),
                lookupFeatureConfigV2.useHeader,
                lookupFeatureConfigV2.useSparse,
                lookupFeatureConfigV2.keyType,
                lookupFeatureConfigV2.valueType);
    } else { // LookupFeatureFunctionVersion::V3 == version
        return new LookupFeatureFunctionV3(
                featureConfig.getFeatureName(),
                normalizer,
                featureConfig.combiner,
                featureConfig.valueDimension,
                featureConfig.getBoundaries());
    }
}

FeatureFunction *FeatureFunctionCreator::createFeatureFunction(
        const SingleFeatureConfig *singleConfig)
{
    const string &type = singleConfig->type;
    if (type == "id_feature") {
        const IdFeatureConfig *idFeatureConfig =
            static_cast<const IdFeatureConfig*>(singleConfig);
        return new IdFeatureFunction(
                idFeatureConfig->getFeatureName(),
                idFeatureConfig->getFeaturePrefix(),
                idFeatureConfig->pruneTo,
                idFeatureConfig->invalidValues);
    } else if (type == "combo_feature") {
        const ComboFeatureConfig *comboFeatureConfig =
            static_cast<const ComboFeatureConfig *>(singleConfig);
        return new ComboFeatureFunction(
                comboFeatureConfig->getFeatureName(),
                comboFeatureConfig->getFeaturePrefix(),
                comboFeatureConfig->pruneRight,
                comboFeatureConfig->pruneLimit,
                comboFeatureConfig->expressions.size(),
                comboFeatureConfig->needSort);
    } else if (type == "raw_feature") {
        const RawFeatureConfig *rawFeatureConfig =
            static_cast<const RawFeatureConfig *>(singleConfig);
        return createRawFeatureFunction(*rawFeatureConfig);
    } else if (type == "match_feature") {
        const MatchFeatureConfig *matchFeatureConfig =
            static_cast<const MatchFeatureConfig *>(singleConfig);
        return createMatchFeatureFunction(*matchFeatureConfig);
    } else if (type == "lookup_feature") {
        const LookupFeatureConfig *lookupFeatureConfig =
            static_cast<const LookupFeatureConfig *>(singleConfig);
        return createLookupFeatureFunction(*lookupFeatureConfig, LookupFeatureFunctionVersion::V1);
    } else if (type == "lookup_feature_v2") {
        const LookupFeatureConfigV2 *lookupFeatureConfigV2 =
            static_cast<const LookupFeatureConfigV2 *>(singleConfig);
        return createLookupFeatureFunction(*lookupFeatureConfigV2, LookupFeatureFunctionVersion::V2);
    } else if (type == "lookup_feature_v3") {
        const LookupFeatureConfigV3 *lookupFeatureConfigV3 =
            static_cast<const LookupFeatureConfigV3 *>(singleConfig);
        return createLookupFeatureFunction(*lookupFeatureConfigV3, LookupFeatureFunctionVersion::V3);
    } else if (type == "lookup_feature_btree") {
        const LookupFeatureConfigBTree *lookupFeatureConfigBTree =
            static_cast<const LookupFeatureConfigBTree *>(singleConfig);
        return createLookupFeatureFunction(*lookupFeatureConfigBTree, LookupFeatureFunctionVersion::BTree);
    } else if (type == "overlap_feature") {
        const OverLapFeatureConfig *overLapFeatureConfig =
            static_cast<const OverLapFeatureConfig*>(singleConfig);
        return new OverLapFeatureFunction(overLapFeatureConfig->getFeatureName(),
                overLapFeatureConfig->getMethod(),
                overLapFeatureConfig->separator,
                overLapFeatureConfig->needDiscrete,
                overLapFeatureConfig->needPrefix,
                overLapFeatureConfig->needDense,
                overLapFeatureConfig->cutThreshold);
    } else if (type == "kgb_match_semantic") {
        const KgbMatchSemanticConfig *kgbMatchSemanticConfig =
            static_cast<const KgbMatchSemanticConfig*>(singleConfig);
        return new KgbMatchSemanticFeatureFunction(kgbMatchSemanticConfig->getFeatureName(),
                kgbMatchSemanticConfig->getFeaturePrefix(),
                kgbMatchSemanticConfig->matchOrnot,
                kgbMatchSemanticConfig->asBytes,
                kgbMatchSemanticConfig->needCombo,
                kgbMatchSemanticConfig->needHitRet,
                kgbMatchSemanticConfig->comboRight);

    } else if (type == "preclick_urb_word_feature") {
        const PreclickUrbWordFeatureConfig *preclickUrbWordFeatureConfig =
            static_cast<const PreclickUrbWordFeatureConfig*>(singleConfig);
        return new PreclickUrbWordFeatureFunction(preclickUrbWordFeatureConfig->getFeatureName(),
                preclickUrbWordFeatureConfig->getFeaturePrefix(),
                preclickUrbWordFeatureConfig->need_decode,
                preclickUrbWordFeatureConfig->need_match,
                preclickUrbWordFeatureConfig->output_count,
                preclickUrbWordFeatureConfig->delim_item,
                preclickUrbWordFeatureConfig->delim_kv,
                preclickUrbWordFeatureConfig->raw_expression,
                preclickUrbWordFeatureConfig->uint64_expression);
    } else {
        AUTIL_LOG(DEBUG, "unknown feature type[%s]!", type.c_str());
        return nullptr;
    }
}

}
