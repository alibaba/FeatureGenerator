#ifndef ISEARCH_FG_LITE_FEATURECONFIG_H
#define ISEARCH_FG_LITE_FEATURECONFIG_H
#include <exception>

#include "autil/Log.h"
#include "autil/legacy/jsonizable.h"
#include "autil/StringUtil.h"

#include "fg_lite/feature/LookupFeatureDataType.h"

namespace fg_lite {

class SingleFeatureConfig : public autil::legacy::Jsonizable
{
public:
    SingleFeatureConfig()
        : valueDimension(1)
        , defaultValue(0.0)
        , needPrefix(true)
        , needDiscrete(true)
    {}
    SingleFeatureConfig(const std::string &t, const std::string &featName)
        : type(t)
        , valueDimension(1)
        , defaultValue(0.0)
        , featureName(featName)
        , needPrefix(true)
        , needDiscrete(true)
    {}
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        json.Jsonize("feature_type", type, type);
        json.Jsonize("feature_name", featureName, featureName);
        json.Jsonize("value_dimension", valueDimension, valueDimension);
        json.Jsonize("sequence_feature_name", sequenceFeatureName, sequenceFeatureName);
        json.Jsonize("need_prefix", needPrefix, needPrefix);
        json.Jsonize("needDiscrete", needDiscrete, needDiscrete);

        if (FROM_JSON == json.GetMode()) {
            json.Jsonize("bucketize_boundaries", boundariesStr, boundariesStr);
            autil::StringUtil::fromString(boundariesStr, boundaries, ",");
            if (!std::is_sorted(boundaries.begin(), boundaries.end())) {
                throw std::invalid_argument("feature[" + getFeatureName() +
                        "] boundaries must be sorted." +getFeatureName());
            }
        } else {
            boundariesStr = autil::StringUtil::toString(boundaries, ",");
            json.Jsonize("bucketize_boundaries", boundariesStr, boundariesStr);
        }
    }
    virtual const std::vector<std::string> getDependInputs() const = 0;
    virtual const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const = 0;
    std::string getFeatureType() const { return type; }
    std::string getFeaturePrefix() const;
    std::string getFeatureName() const;
    const std::vector<float> & getBoundaries() const {
        return boundaries;
    }
public:
    static SingleFeatureConfig* create(const std::string &featureConfigJson);
    static SingleFeatureConfig* create(const autil::legacy::json::JsonMap &jsonMap);
public:
    std::string type;
    int32_t valueDimension;
    float defaultValue;
    std::string featureName;
    bool needPrefix;
    bool needDiscrete;
private:
    std::string boundariesStr;
    std::string sequenceFeatureName;
    std::vector<float> boundaries;
protected:
    AUTIL_LOG_DECLARE();
};

class KgbMatchSemanticConfig : public SingleFeatureConfig {
public:
    KgbMatchSemanticConfig()
        : SingleFeatureConfig("kgb_match_semantic", "")
        , matchOrnot(true)
        , asBytes(false)
        , needCombo(false)
        , needHitRet(false)
        , comboRight(false)
    {
    }

    KgbMatchSemanticConfig(const std::string& featName)
       : SingleFeatureConfig("kgb_match_semantic", featName)
        , matchOrnot(true)
        , asBytes(false)
        , needCombo(false)
        , needHitRet(false)
        , comboRight(false)
    {
    }

    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("queryTermListExpression", queryTermListExpression, queryTermListExpression);
        json.Jsonize("itemTermListExpression", itemTermListExpression, itemTermListExpression);
        json.Jsonize("otherListExpression", otherListExpression, otherListExpression);
        json.Jsonize("matchOrnot", matchOrnot, matchOrnot);
        json.Jsonize("asBytes", asBytes, asBytes);
        json.Jsonize("needHitRet", needHitRet, needHitRet);
        json.Jsonize("comboRight", comboRight, comboRight);
        //json.Jsonize("needDiscrete", needDiscrete, needDiscrete);
    }

    const std::vector<std::string> getDependInputs() const override {
        return {queryTermListExpression, itemTermListExpression};
    }

    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        if (needCombo) {
            return {
                {"queryTermListExpression", queryTermListExpression},
                {"itemTermListExpression", itemTermListExpression},
                {"otherListExpression", otherListExpression}
            };
        }
        return {
            {"queryTermListExpression", queryTermListExpression},
            {"itemTermListExpression", itemTermListExpression}
        };
    }

public:
    std::string queryTermListExpression;
    std::string itemTermListExpression;
    std::string otherListExpression;
    bool matchOrnot;
    bool asBytes;
    bool needCombo;
    bool needHitRet;
    bool comboRight;
};

class IdFeatureConfig : public SingleFeatureConfig {
public:
    IdFeatureConfig()
        : SingleFeatureConfig("id_feature", "")
    {
    }
    IdFeatureConfig(const std::string &featName,
                    const std::string &expr)
        : SingleFeatureConfig("id_feature", featName)
        , expression(expr)
    {
    }
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("expression", expression, expression);
        json.Jsonize("shared_embedding_name", sharedEmbeddingName, sharedEmbeddingName);
        json.Jsonize("force_prefix_name", forcePrefixName, forcePrefixName);
        json.Jsonize("force_prefix_name", forcePrefixName, forcePrefixName);
        json.Jsonize("prune_to", pruneTo, pruneTo);
        json.Jsonize("invalid_values", invalidValues, invalidValues);
        if (!sharedEmbeddingName.empty()) {
            if (!forcePrefixName.empty()) {
                featureName = forcePrefixName;
            } else {
                needPrefix = false;
            }
        }
    }
    const std::vector<std::string> getDependInputs() const override {
        return {expression};
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        return {{"expression", expression}};
    }
public:
    std::string expression;
    std::string sharedEmbeddingName;
    std::string forcePrefixName;
    int32_t pruneTo = std::numeric_limits<int>::max();
    std::vector<std::string> invalidValues;
};

class LookupFeatureConfig : public SingleFeatureConfig {
public:
    LookupFeatureConfig()
        : combiner("sum")
        , combiner2("none")
        , needKey(true)
        , needWeighting(false)
        , isOptimized(false)
        , hasDefault(false)
        , timediff(-1.0f)
        , needCombo(false)
        , countCutThreshold(-1)
        , count2CutThreshold(-1)
        , comboRight(true)
        , comboSimple(false)
    {
        type = "lookup_feature";
        defaultLookupValue = "";
    }
    ~LookupFeatureConfig()  = default;
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("key", keyExpression, keyExpression);
        json.Jsonize("map", mapExpression, mapExpression);
        json.Jsonize("map_keys", mapKeysExpression, mapKeysExpression);
        json.Jsonize("map2_keys", map2KeysExpression, map2KeysExpression);
        json.Jsonize("map_values", mapValuesExpression, mapValuesExpression);
        json.Jsonize("map2_values", map2ValuesExpression, map2ValuesExpression);
        json.Jsonize("map_encode", encodedMapExpression, encodedMapExpression);
        json.Jsonize("value_timestamps", valueTimestampExpression, valueTimestampExpression);
        json.Jsonize("value2_timestamps", value2TimestampExpression, value2TimestampExpression);
        json.Jsonize("key_timestamps", timestampExpression, timestampExpression);
        json.Jsonize("normalizer", normalizer, normalizer);
        json.Jsonize("combiner", combiner, combiner);
        json.Jsonize("combiner2", combiner2, combiner2);
        json.Jsonize("needKey", needKey, needKey);
        json.Jsonize("needWeighting", needWeighting, needWeighting);
        json.Jsonize("is_optimized", isOptimized, isOptimized);
        json.Jsonize("timediff", timediff, timediff);
        json.Jsonize("countCutThreshold", countCutThreshold, countCutThreshold);
        json.Jsonize("count2CutThreshold", count2CutThreshold, count2CutThreshold);
        json.Jsonize("comboRight", comboRight, comboRight);
        json.Jsonize("comboSimple", comboSimple, comboSimple);
        json.Jsonize("otherExpression", otherExpression, otherExpression);
        json.Jsonize("defaultLookupValue", defaultLookupValue, defaultLookupValue);
        //json.Jsonize("needDiscrete", needDiscrete, needDiscrete);

        if (FROM_JSON == json.GetMode()) {
            try {
                json.Jsonize("default_lookup_value", defaultLookupValue);
                hasDefault = true;
            } catch (const autil::legacy::ExceptionBase &) {
                hasDefault = false;
            }
        } else if (hasDefault) {
            json.Jsonize("default_lookup_value", defaultLookupValue);
        }
        if (needDiscrete && needKey && isOptimized) {
            throw std::invalid_argument("the input is optimized but needDiscrete and needKey");
        }
    }
    const std::vector<std::string> getDependInputs() const override {
        if (timediff >= 0.0 && !valueTimestampExpression.empty() && !timestampExpression.empty()) {
            if (!needCombo) {
                return {mapKeysExpression, mapValuesExpression, keyExpression, timestampExpression, valueTimestampExpression};
            } else {
                if (comboSimple) {
                    return {mapKeysExpression, mapValuesExpression, keyExpression, timestampExpression, valueTimestampExpression,
                        otherExpression
                    };

                }
                return {mapKeysExpression, mapValuesExpression, keyExpression, timestampExpression, valueTimestampExpression,
                    map2KeysExpression, map2ValuesExpression, value2TimestampExpression
                };
            }
        }
        if (!mapValuesExpression.empty() && !mapKeysExpression.empty()) {
            return {mapKeysExpression, mapValuesExpression, keyExpression};
        }
        return {mapExpression, keyExpression};
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        if (timediff >= 0.0 && !valueTimestampExpression.empty() && !timestampExpression.empty()) {
            if (!needCombo) {
                return {
                    {"map_keys", mapKeysExpression},
                    {"map_values", mapValuesExpression},
                    {"key", keyExpression},
                    {"key_timestamps",timestampExpression},
                    {"value_timestamps", valueTimestampExpression}
                };
            } else {
                if (comboSimple) {
                    return {
                        {"map_keys", mapKeysExpression},
                            {"map_values", mapValuesExpression},
                            {"key", keyExpression},
                            {"key_timestamps",timestampExpression},
                            {"value_timestamps", valueTimestampExpression},
                            {"otherExpression", otherExpression}
                    };
                }
                return {
                    {"map_keys", mapKeysExpression},
                    {"map_values", mapValuesExpression},
                    {"key", keyExpression},
                    {"key_timestamps",timestampExpression},
                    {"value_timestamps", valueTimestampExpression},
                    {"map2_keys", map2KeysExpression},
                    {"map2_values", map2ValuesExpression},
                    {"value2_timestamps", value2TimestampExpression}
                };
            }
        }
        if (!mapValuesExpression.empty() && !mapKeysExpression.empty()) {
            return {
                {"map_keys", mapKeysExpression},
                {"map_values", mapValuesExpression},
                {"key", keyExpression}
            };
        }
        return {
            {"map", mapExpression},
                {"key", keyExpression}
        };
    }
public:
    std::string keyExpression;
    std::string mapExpression;
    std::string encodedMapExpression;
    std::string mapKeysExpression;
    std::string mapValuesExpression;
    std::string map2KeysExpression;
    std::string map2ValuesExpression;
    std::string valueTimestampExpression;
    std::string value2TimestampExpression;
    std::string timestampExpression;
    std::string otherExpression;
    std::string normalizer;
    std::string combiner;
    std::string combiner2;
    std::string defaultLookupValue;
    bool needKey;
    bool needWeighting;
    bool isOptimized;
    bool hasDefault;
    float timediff;
    bool needCombo;
    int countCutThreshold;
    int count2CutThreshold;
    bool comboRight;
    bool comboSimple;
};

class LookupFeatureConfigV2 : public LookupFeatureConfig {
public:
    LookupFeatureConfigV2() {
        type = "lookup_feature_v2";
    }

    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        LookupFeatureConfig::Jsonize(json);
        json.Jsonize("use_header", useHeader, useHeader);
        json.Jsonize("use_sparse", useSparse, useSparse);
        json.Jsonize("min_hash_type", keyType, keyType);
        json.Jsonize("value_encode_type", valueType, valueType);
    }
public:
    bool useHeader = false;
    bool useSparse = false;
    LookupFeatureV3KeyType keyType = LOOKUP_V3_KEY_HASH_0_TO_63_BIT;
    LookupFeatureV3ValueType valueType = LOOKUP_V3_VALUE_ENCODE_32BIT;
};

class LookupFeatureConfigV3 : public LookupFeatureConfig {
public:
    LookupFeatureConfigV3() {
        type = "lookup_feature_v3";
    }

    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        LookupFeatureConfig::Jsonize(json);

        if (json.GetMode() == FROM_JSON) {
            json.Jsonize("bucketize_multi_boundaries", multiBoundariesStr, multiBoundariesStr);
            autil::StringUtil::fromString(multiBoundariesStr, multiBoundaries, ",", "|");
            for (const auto &boundaries : multiBoundaries) {
                if (!std::is_sorted(boundaries.begin(), boundaries.end())) {
                    throw std::invalid_argument("feature[" + getFeatureName() +
                            "] boundaries must be sorted." +getFeatureName());
                }
            }
        } else {
            multiBoundariesStr = autil::StringUtil::toString(multiBoundaries, ",", "|");
            json.Jsonize("bucketize_multi_boundaries", multiBoundariesStr, multiBoundariesStr);
        }
    }
public:
    std::vector<std::vector<float>> multiBoundaries;
    std::string multiBoundariesStr;
};

class LookupFeatureConfigBTree : public LookupFeatureConfigV3 {
public:
    LookupFeatureConfigBTree() {
        type = "lookup_feature_btree"; // need to be lowercase
    }
};

class ComboFeatureConfig : public SingleFeatureConfig {
public:
    ComboFeatureConfig()
        : SingleFeatureConfig("combo_feature", "")
        , needSort(false)
    {
    }
    ComboFeatureConfig(const std::string &featName)
        : SingleFeatureConfig("combo_feature", featName)
        , needSort(false)
    {
    }
    ~ComboFeatureConfig()  = default;
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("expression", expressions, expressions);
        json.Jsonize("pruneRight", pruneRight, pruneRight);
        json.Jsonize("pruneLimit", pruneLimit, pruneLimit);
        json.Jsonize("needSort", needSort, needSort);
        if (pruneRight.size() < expressions.size()) {
            std::fill_n(std::back_inserter(pruneRight), expressions.size() - pruneRight.size(), std::numeric_limits<int>::max());
        }
        if (pruneLimit.size() < expressions.size()) {
            std::fill_n(std::back_inserter(pruneLimit), expressions.size() - pruneLimit.size(), std::numeric_limits<int>::max());
        }
        for (auto len : pruneLimit) {
            if (len <= 1) {
                throw std::invalid_argument("pruneLimit must > 1, " + getFeatureName());
            }
        }
    }
    const std::vector<std::string> getDependInputs() const override {
        return expressions;
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        std::vector<std::pair<std::string, std::string>> inputs;
        for (auto& val: expressions) {
            inputs.push_back({"expression", val});
        }
        return inputs;
    }
public:
    std::vector<std::string> expressions;
    std::vector<bool> pruneRight;
    std::vector<int32_t> pruneLimit;
    bool needSort;
};

class MatchFeatureConfig : public SingleFeatureConfig {
public:
    MatchFeatureConfig() {
        type = "match_feature";
        needWeighting = false;
        showCategory = true;
        showItem = true;
    }
    MatchFeatureConfig(const std::string &featName)
        : SingleFeatureConfig("match_feature", featName)
    {
        needWeighting = false;
        showCategory = true;
        showItem = true;
    }
    ~MatchFeatureConfig()  = default;
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("category", categoryExpression, categoryExpression);
        json.Jsonize("user", userExpression, userExpression);
        json.Jsonize("item", itemExpression, itemExpression);
        json.Jsonize("matchType", matchType, matchType);
        //json.Jsonize("needDiscrete", needDiscrete, needDiscrete);
        json.Jsonize("needWeighting", needWeighting, needWeighting);
        json.Jsonize("showCategory", showCategory, showCategory);
        json.Jsonize("showItem", showItem, showItem);
        json.Jsonize("normalizer", normalizer, normalizer);
    }
    const std::vector<std::string> getDependInputs() const override {
        std::vector<std::string> inputs;
        inputs.push_back(userExpression);
        if (itemExpression != "ALL") {
            inputs.push_back(itemExpression);
        }
        if (categoryExpression != "ALL") {
            inputs.push_back(categoryExpression);
        }
        return inputs;
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        std::vector<std::pair<std::string,std::string>> inputs;
        inputs.push_back({"user", userExpression});
        if (itemExpression != "ALL") {
            inputs.push_back({"item", itemExpression});
        }
        if (categoryExpression != "ALL") {
            inputs.push_back({"category", categoryExpression});
        }
        return inputs;
    }
public:
    std::string categoryExpression; // can be ALL
    std::string userExpression; // type of userExpression should be string, valid format is: catId1^featName1:w1,featName2:w2|catId2^w1:w1,featName2:w2
    std::string itemExpression; // can be ALL
    std::string matchType; // supported match function: hit, hit1, range, cos, cross
    std::string normalizer;
    bool needWeighting;
    bool showCategory;
    bool showItem;
};

class RawFeatureConfig : public SingleFeatureConfig {
public:
    RawFeatureConfig() {
        type = "raw_feature";
        needDiscrete = false;
    }
    RawFeatureConfig(const std::string &featName, const std::string &expr)
        : SingleFeatureConfig("raw_feature", featName)
        , expression(expr) {
            needDiscrete = false;
        }
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);

        json.Jsonize("expression", expression, expression);
        json.Jsonize("normalizer", normalizer, normalizer);
        if (FROM_JSON == json.GetMode()) {
            try {
                json.Jsonize("default_value", defaultValue, defaultValue);
            } catch (const std::exception &e) {
                std::string defaultValueStr;
                json.Jsonize("default_value", defaultValueStr, defaultValueStr);
                defaultValue = autil::StringUtil::fromString<float>(defaultValueStr);
            }
        } else {
            json.Jsonize("default_value", defaultValue);
        }
    }
    const std::vector<std::string> getDependInputs() const override {
        return {expression};
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        return {{"expression", expression}};
    }

public:
    std::string expression;
    std::string normalizer;
};

class GBDTFeatureConfig : public SingleFeatureConfig {
public:
    GBDTFeatureConfig() {
        type = "gbdt_feature";
    }
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("feature_conf_file", featureConfFile, featureConfFile);
        json.Jsonize("model_table", modelTable, modelTable);
    }
    const std::string &getDependentFeatureFile() const {
        return featureConfFile;
    }
    // not supported in new java fg job
    const std::vector<std::string> getDependInputs() const override {
        return {};
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        return {};
    }
public:
    std::string featureConfFile;
    std::string modelTable;
};

class OverLapFeatureConfig : public SingleFeatureConfig {
public:
    enum OverLapType {
        OT_EQUAL,
        OT_CONTAIN,
        OT_COMMON_WORDS,
        OT_DIFF_WORDS,
        OT_COMMON_WORDS_DIVIDED,
        OT_DIFF_WORDS_DIVIDED,
        OT_QUERY_RATIO,
        OT_TITLE_RATIO,
        OT_DIFF_BOTH,
        OT_MATCH_WORDS,
        OT_MATCH_WORDS_DIVIDED,
        OT_HIT_ORNOT,
        OT_INVALID
    };
    OverLapFeatureConfig(): SingleFeatureConfig() {
        type = "overlap_feature";
        method = "is_contain";
        separator = "_";
        needDense = false;
        property = "";
        cutThreshold = -1;
    }
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("query", query, query);
        json.Jsonize("title", title, title);
        json.Jsonize("method", method, method);
        json.Jsonize("separator", separator, separator);
        //json.Jsonize("needDiscrete", needDiscrete, needDiscrete);
        json.Jsonize("property", property, property);
        json.Jsonize("needDense", needDense, needDense);
        json.Jsonize("cutThreshold", cutThreshold, cutThreshold);

    }
    const std::vector<std::string> getDependInputs() const override {
        if (!property.empty()) {
            return {query, title, property};
        }
        return {query, title};
    }

    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        if (!property.empty()) {
            return {{"query", query}, {"title", title}, {"property", property}};
        }
        return {{"query", query}, {"title", title}};
    }

    OverLapType getMethod() const {
        return stringToType(method);
    }

    static OverLapType stringToType(const std::string &method) {
        if (method == "is_equal") {
            return OT_EQUAL;
        } else if (method == "is_contain") {
            return OT_CONTAIN;
        } else if (method == "common_word") {
            return OT_COMMON_WORDS;
        } else if (method == "diff_word") {
            return OT_DIFF_WORDS;
        } else if (method == "common_word_divided") {
            return OT_COMMON_WORDS_DIVIDED;
        } else if (method == "diff_word_divided") {
            return OT_DIFF_WORDS_DIVIDED;
        } else if (method == "query_common_ratio") {
            return OT_QUERY_RATIO;
        } else if (method == "title_common_ratio") {
            return OT_TITLE_RATIO;
        } else if (method == "diff_both") {
            return OT_DIFF_BOTH;
        } else if (method == "match_words") {
            return OT_MATCH_WORDS;
        } else if (method == "match_words_divided") {
            return OT_MATCH_WORDS_DIVIDED;
        } else if (method == "hit_ornot") {
            return OT_HIT_ORNOT;
        } else {
            AUTIL_LOG(ERROR, "Invalid overlap method [%s]", method.c_str());
            return OT_INVALID;
        }
    }
public:
    std::string query;
    std::string title;
    std::string property;
    std::string method;
    std::string separator;
    bool needDense;
    int cutThreshold;
};

class PreclickUrbWordFeatureConfig : public SingleFeatureConfig {
public:
    PreclickUrbWordFeatureConfig()
            : SingleFeatureConfig("preclick_urb_word_feature", "")
            , expression("")
            , need_match(false)
            , match_input("")
            , output_count(false)
            , delim_item(";")
            , delim_kv("")
            , need_decode(true)
            , raw_expression(false)
            , uint64_expression(false)
    {
    }

    PreclickUrbWordFeatureConfig(const std::string &featName,
                    const std::string &expr)
            : SingleFeatureConfig("preclick_urb_word_feature", featName)
            , expression(expr)
            , need_match(false)
            , match_input("")
            , output_count(false)
            , delim_item(";")
            , delim_kv("")
            , need_decode(true)
            , raw_expression(false)
            , uint64_expression(false)
    {
    }
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override {
        SingleFeatureConfig::Jsonize(json);
        json.Jsonize("expression", expression, expression);
        json.Jsonize("need_match", need_match, need_match);
        json.Jsonize("match_input", match_input, match_input);
        json.Jsonize("output_count", output_count, output_count);
        json.Jsonize("delim_item", delim_item, delim_item);
        json.Jsonize("delim_kv", delim_kv, delim_kv);
        json.Jsonize("need_decode", need_decode, need_decode);
        json.Jsonize("raw_expression", raw_expression, raw_expression);
        json.Jsonize("uint64_expression", uint64_expression, uint64_expression);
    }
    const std::vector<std::string> getDependInputs() const override {
        std::vector<std::string> inputs;
        inputs.push_back(expression);
        if (need_match) {
            inputs.push_back(match_input);
        }
        return inputs;
    }
    const std::vector<std::pair<std::string, std::string>> getDependInputsWithKeys() const override {
        std::vector<std::pair<std::string, std::string>> inputs;
        inputs.push_back({"expression", expression});
        if (need_match) {
            inputs.push_back({"match_input", match_input});
        }
        return inputs;
    }
public:
    std::string expression;
    bool need_match;
    std::string match_input;
    bool output_count;
    std::string delim_item;
    std::string delim_kv;
    bool need_decode;
    bool raw_expression;
    bool uint64_expression;
};

class FeatureConfig : public autil::legacy::Jsonizable
{
public:
    FeatureConfig();
    ~FeatureConfig();
public:
    void Jsonize(autil::legacy::Jsonizable::JsonWrapper& json) override;
public:
    std::vector<SingleFeatureConfig*> _featureConfigs;
private:
    AUTIL_LOG_DECLARE();
};


}

#endif //ISEARCH_FG_LITE_FEATURECONFIG_H
