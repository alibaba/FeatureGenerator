#include "fg_lite/feature/FeatureConfig.h"

using namespace std;
using namespace autil::legacy;
namespace fg_lite {

AUTIL_LOG_SETUP(fg_lite, SingleFeatureConfig);
AUTIL_LOG_SETUP(fg_lite, FeatureConfig);

SingleFeatureConfig* SingleFeatureConfig::create(const std::string &featureConfigJson) {
    json::JsonMap jsonMap;
    try {
        FromJsonString(jsonMap, featureConfigJson);
        return create(jsonMap);
    } catch (const exception &e) {
        AUTIL_LOG(ERROR, "parse [%s] failed, %s", featureConfigJson.c_str(), e.what());
        return nullptr;
    } catch (...) {
        AUTIL_LOG(ERROR, "parse [%s] failed, unknown exception", featureConfigJson.c_str());
        return nullptr;
    }
}

SingleFeatureConfig* SingleFeatureConfig::create(
        const autil::legacy::json::JsonMap &jsonMap)
{
    auto it = jsonMap.find("feature_type");
    if (jsonMap.end() == it) {
        AUTIL_LOG(ERROR, "feature type must be specified in %s",
                  ToJsonString(jsonMap).c_str());
        return nullptr;
    }
    const std::string &type = AnyCast<string>(it->second);
    unique_ptr<SingleFeatureConfig> feature;
    if (type == "id_feature") {
        feature.reset(new IdFeatureConfig);
    } else if (type == "combo_feature") {
        feature.reset(new ComboFeatureConfig());
    } else if (type == "raw_feature") {
        feature.reset(new RawFeatureConfig());
    } else if (type == "lookup_feature") {
        feature.reset(new LookupFeatureConfig());
    } else if (type == "lookup_feature_v2") {
        feature.reset(new LookupFeatureConfigV2());
    } else if (type == "lookup_feature_v3") {
        feature.reset(new LookupFeatureConfigV3());
    } else if (type == "match_feature") {
        feature.reset(new MatchFeatureConfig());
    } else if (type == "gbdt_feature") {
        feature.reset(new GBDTFeatureConfig());
    } else if (type == "overlap_feature") {
        feature.reset(new OverLapFeatureConfig());
    } else if (type == "kgb_match_semantic") {
        feature.reset(new KgbMatchSemanticConfig());
    } else if (type == "preclick_urb_word_feature"){
        feature.reset(new PreclickUrbWordFeatureConfig());
    } else {
        AUTIL_LOG(ERROR, "Invalid feature type %s", type.c_str());
        return nullptr;
    }
    autil::legacy::Jsonizable::JsonWrapper json(jsonMap);
    feature->Jsonize(json);
    return feature.release();
}

std::string SingleFeatureConfig::getFeaturePrefix() const {
    if (needPrefix) {
        return featureName + '_';
    }
    return std::string();
}

std::string SingleFeatureConfig::getFeatureName() const {
    if (!sequenceFeatureName.empty()) {
        return sequenceFeatureName;
    }
    return featureName;
}

FeatureConfig::FeatureConfig() {
}

FeatureConfig::~FeatureConfig() {
    for (size_t i = 0; i < _featureConfigs.size(); ++i) {
        delete _featureConfigs[i];
    }
    _featureConfigs.clear();
}

void FeatureConfig::Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) {
    if (json.GetMode() == TO_JSON) {
        std::vector<Any> vec;
        for (size_t i = 0; i < _featureConfigs.size(); ++i) {
            vec.push_back(ToJson(*_featureConfigs[i]));
        }
        json.Jsonize("features", vec, vec);
    } else {
        std::vector<Any> vec;
        json.Jsonize("features", vec, vec);
        for (size_t i = 0; i < vec.size(); ++i) {
            const auto &anyMap = AnyCast<json::JsonMap>(vec[i]);
            auto featureConfig = SingleFeatureConfig::create(anyMap);
            _featureConfigs.push_back(featureConfig);
        }
        json.Jsonize("features", vec, vec);
    }
}

}
