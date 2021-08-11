#include "fg_lite/feature/MatchFunctionImpl.h"
#include "fg_lite/feature/FeatureFormatter.h"
#include "autil/StringUtil.h"
#include "autil/StringTokenizer.h"

using namespace std;
using namespace autil;

namespace fg_lite {

/////////////////////////////////////////////////////////////////////////////
AUTIL_LOG_SETUP(fg_lite, HitMatch);

const char HitMatch::HIT_SEPARATOR = '_';
HitMatch::HitMatch(const string &featNamePrefix, bool needDiscrete, bool needWeighting)
    : MatchFunction(featNamePrefix, needDiscrete, true, true, needWeighting)
{}

void HitMatch::matchOneFeature(const ConstString &category,
                               const ConstString &itemValue,
                               const UserMatchInfo &userMatchInfo,
                               MultiDenseFeatures *features) const
{
    string matchedValue;
    if (!userMatchInfo.match(category, itemValue, matchedValue)) {
        return;
    }
    double featureValue = StringUtil::fromString<double>(matchedValue);
    featureValue = _normalizer.normalize(featureValue);
    features->addFeatureValue(featureValue);
}

void HitMatch::matchOneFeature(const ConstString &category,
                               const ConstString &itemValue,
                               const UserMatchInfo &userMatchInfo,
                               MultiSparseFeatures *features) const
{
    string matchedValue;
    if (!userMatchInfo.match(category, itemValue, matchedValue)) {
        return;
    }
    FeatureFormatter::FeatureBuffer buffer(cp_alloc(features->getPool()));
    buffer.assign(_featureNamePrefix.begin(), _featureNamePrefix.end());
    buffer.push_back(HIT_SEPARATOR);
    buffer.insert(buffer.end(), category.begin(), category.end());
    buffer.push_back(HIT_SEPARATOR);
    buffer.insert(buffer.end(), itemValue.begin(), itemValue.end());
    buffer.push_back(HIT_SEPARATOR);
    buffer.insert(buffer.end(), matchedValue.begin(), matchedValue.end());
    features->addFeatureKey(buffer.data(), buffer.size());
}

void HitMatch::matchOneFeature(const ConstString &category,
                               const ConstString &itemValue,
                               const UserMatchInfo &userMatchInfo,
                               MultiSparseWeightingFeatures *features) const
{
    string matchedValue;
    if (!userMatchInfo.match(category, itemValue, matchedValue)) {
        return;
    }

    double featureValue = StringUtil::fromString<double>(matchedValue);
    featureValue = _normalizer.normalize(featureValue);
    features->addFeatureValue(featureValue);

    FeatureFormatter::FeatureBuffer buffer(cp_alloc(features->getPool()));
    buffer.assign(_featureNamePrefix.begin(), _featureNamePrefix.end());
    buffer.push_back(HIT_SEPARATOR);
    buffer.insert(buffer.end(), category.begin(), category.end());
    buffer.push_back(HIT_SEPARATOR);
    buffer.insert(buffer.end(), itemValue.begin(), itemValue.end());
    features->addFeatureKey(buffer.data(), buffer.size());
}

/////////////////////////////////////////////////////////////////////////////
AUTIL_LOG_SETUP(fg_lite, MultiHitMatch);
const char MultiHitMatch::HIT_SEPARATOR = '_';

MultiHitMatch::MultiHitMatch(const string &featNamePrefix, bool needDiscrete,
                             bool showCategory, bool showItem, bool needWeighting)
    : MatchFunction(featNamePrefix, needDiscrete, showCategory, showItem, needWeighting)
{}

MultiHitMatch::~MultiHitMatch()  = default;

void MultiHitMatch::matchOneFeature(const ConstString &category,
                                    const ConstString &itemValue,
                                    const UserMatchInfo &userMatchInfo,
                                    MultiDenseFeatures *features) const
{
    vector<MatchResultUnit> matchedValues;
    if (!userMatchInfo.match(category, itemValue, matchedValues)) {
        return;
    }

    for (size_t i = 0; i < matchedValues.size(); ++i) {
        double featureValue = StringUtil::fromString<double>(
                matchedValues[i]._value.toString());
        features->addFeatureValue(featureValue);
    }
}

void MultiHitMatch::matchOneFeature(const ConstString &category,
                                    const ConstString &itemValue,
                                    const UserMatchInfo &userMatchInfo,
                                    MultiSparseFeatures *features) const
{
    vector<MatchResultUnit> matchedValues;
    if (!userMatchInfo.match(category, itemValue, matchedValues)) {
        return;
    }

    FeatureFormatter::FeatureBuffer buffer(cp_alloc(features->getPool()));
    buffer.assign(_featureNamePrefix.begin(), _featureNamePrefix.end());
    size_t pos = buffer.size();
    for (size_t i = 0; i < matchedValues.size(); ++i) {
        if (_showCategory) {
            buffer.push_back(HIT_SEPARATOR);
            buffer.insert(buffer.end(),
                          matchedValues[i]._category.begin(),
                          matchedValues[i]._category.end());
        }
        if (_showItem) {
            buffer.push_back(HIT_SEPARATOR);
            buffer.insert(buffer.end(),
                          matchedValues[i]._item.begin(),
                          matchedValues[i]._item.end());
        }
        buffer.push_back(HIT_SEPARATOR);
        buffer.insert(buffer.end(),
                      matchedValues[i]._value.begin(),
                      matchedValues[i]._value.end());
        features->addFeatureKey(buffer.data(), buffer.size());
        buffer.erase(buffer.begin() + pos, buffer.end());
    }
}

void MultiHitMatch::matchOneFeature(const ConstString &category,
                                    const ConstString &itemValue,
                                    const UserMatchInfo &userMatchInfo,
                                    MultiSparseWeightingFeatures *features) const
{
    vector<MatchResultUnit> matchedValues;
    if (!userMatchInfo.match(category, itemValue, matchedValues)) {
        return;
    }

    for (size_t i = 0; i < matchedValues.size(); ++i) {
        double featureValue = StringUtil::fromString<double>(
                matchedValues[i]._value.toString());
        features->addFeatureValue(featureValue);
    }

    FeatureFormatter::FeatureBuffer buffer(cp_alloc(features->getPool()));
    buffer.assign(_featureNamePrefix.begin(), _featureNamePrefix.end());
    size_t pos = buffer.size();
    for (size_t i = 0; i < matchedValues.size(); ++i) {
        if (_showCategory) {
            buffer.push_back(HIT_SEPARATOR);
            buffer.insert(buffer.end(),
                          matchedValues[i]._category.begin(),
                          matchedValues[i]._category.end());
        }
        if (_showItem) {
            buffer.push_back(HIT_SEPARATOR);
            buffer.insert(buffer.end(),
                          matchedValues[i]._item.begin(),
                          matchedValues[i]._item.end());
        }
        buffer.push_back(HIT_SEPARATOR);
        buffer.insert(buffer.end(),
                      matchedValues[i]._value.begin(),
                      matchedValues[i]._value.end());
        features->addFeatureKey(buffer.data(), buffer.size());
        buffer.erase(buffer.begin() + pos, buffer.end());
    }
}

///////////////////////////////////////////////////////////////////////////////////
AUTIL_LOG_SETUP(fg_lite, CrossMatch);
const char CrossMatch::CROSS_SEPARATOR = '_';

CrossMatch::CrossMatch(const string &featNamePrefix)
    : MatchFunction(featNamePrefix, true, false, true)
{}

CrossMatch::~CrossMatch()  = default;

void CrossMatch::matchOneFeature(const ConstString &category,
                                 const ConstString &itemValue,
                                 const UserMatchInfo &userMatchInfo,
                                 MultiDenseFeatures *features) const
{
    assert(false);
    // do not support
}

void CrossMatch::matchOneFeature(const ConstString &category,
                                 const ConstString &itemValue,
                                 const UserMatchInfo &userMatchInfo,
                                 MultiSparseFeatures *features) const
{
    vector<ConstString> matchedKeys;
    if (!userMatchInfo.matchCategoryForKeys(category, matchedKeys)) {
        return;
    }
    FeatureFormatter::FeatureBuffer buffer(cp_alloc(features->getPool()));
    buffer.assign(_featureNamePrefix.begin(), _featureNamePrefix.end());
    buffer.push_back(CROSS_SEPARATOR);
    buffer.insert(buffer.end(), category.begin(), category.end());
    buffer.push_back(CROSS_SEPARATOR);
    for(size_t i = 0; i < matchedKeys.size(); i++) {
        size_t tempPos = buffer.size();
        buffer.insert(buffer.end(),
                      matchedKeys[i].begin(),
                      matchedKeys[i].end());
        buffer.push_back(CROSS_SEPARATOR);
        buffer.insert(buffer.end(), itemValue.begin(), itemValue.end());
        features->addFeatureKey(buffer.data(), buffer.size());
        buffer.erase(buffer.begin() + tempPos, buffer.end());
    }
}

void CrossMatch::matchOneFeature(const ConstString &category,
                                 const ConstString &itemValue,
                                 const UserMatchInfo &userMatchInfo,
                                 MultiSparseWeightingFeatures *features) const
{
    assert(false);
    // do not support
}

///////////////////////////////////////////////////////////////////////////////////////////////
AUTIL_LOG_SETUP(fg_lite, CosMatch);
const std::string CosMatch::SEPARATOR_BETWEEN_KV = ",";
const std::string CosMatch::SEPARATOR_KV = ":";
const char CosMatch::COS_SEPARATOR = '_';

CosMatch::CosMatch(const string &featNamePrefix, bool needDiscrete)
    : MatchFunction(featNamePrefix, needDiscrete, false, true)
{}

CosMatch::~CosMatch()  = default;

void CosMatch::matchOneFeature(const ConstString &category,
                               const ConstString &itemValue,
                               const UserMatchInfo &userMatchInfo,
                               MultiDenseFeatures *features) const
{
    vector<ConstString> keys;
    vector<int64_t> values;
    if (!parseItemValueForCos(itemValue, keys, values)) {
        AUTIL_LOG(WARN, "item value has invalid format.");
        return;
    }
    vector<int64_t> matchedValues;
    int32_t matchedCount = 0;
    if (!userMatchInfo.batchMatchForValues(category, keys,
                    matchedValues, matchedCount))
    {
        AUTIL_LOG(WARN, "no match for [%s], [%s]", category.toString().c_str(), itemValue.toString().c_str());
        return;
    }
    int64_t featureValue = calculateFeatureValue(values, matchedValues, matchedCount);
    features->addFeatureValue(featureValue);
}

void CosMatch::matchOneFeature(const ConstString &category,
                               const ConstString &itemValue,
                               const UserMatchInfo &userMatchInfo,
                               MultiSparseFeatures *features) const
{
    vector<ConstString> keys;
    vector<int64_t> values;
    if (!parseItemValueForCos(itemValue, keys, values)) {
        AUTIL_LOG(WARN, "item value has invalid format.");
        return;
    }
    vector<int64_t> matchedValues;
    int32_t matchedCount = 0;
    if (!userMatchInfo.batchMatchForValues(category, keys,
                    matchedValues, matchedCount))
    {
        AUTIL_LOG(WARN, "no match for [%s], [%s]", category.toString().c_str(), itemValue.toString().c_str());
        return;
    }
    int64_t featureValue = calculateFeatureValue(values, matchedValues, matchedCount);
    FeatureFormatter::FeatureBuffer buffer(cp_alloc(features->getPool()));
    buffer.assign(_featureNamePrefix.begin(), _featureNamePrefix.end());
    buffer.push_back(COS_SEPARATOR);
    FeatureFormatter::fillFeatureToBuffer(featureValue, buffer);
    features->addFeatureKey(buffer.data(), buffer.size());
}

void CosMatch::matchOneFeature(const ConstString &category,
                               const ConstString &itemValue,
                               const UserMatchInfo &userMatchInfo,
                               MultiSparseWeightingFeatures *features) const
{
    assert(false);
    // do not support
}

int64_t CosMatch::calculateFeatureValue(const vector<int64_t> &values1,
                                        const vector<int64_t> &values2,
                                        int32_t matchedCount) const
{
    assert(values1.size() == values2.size());

    int64_t result = 0;
    if (matchedCount == 0) {
        return result;
    }
    for (size_t i = 0; i < values1.size(); i++) {
        result += values1[i] * values2[i];
    }
    result = (int64_t)((float)result / (100 * 100 * matchedCount) * 100);
    return result;
}

bool CosMatch::parseItemValueForCos(const ConstString &itemValue, vector<ConstString> &keys,
                                    vector<int64_t> &values) const
{
    StringTokenizer::Option opt = StringTokenizer::TOKEN_IGNORE_EMPTY | StringTokenizer::TOKEN_TRIM;
    auto kvStrs = StringTokenizer::constTokenize(itemValue,
            SEPARATOR_BETWEEN_KV, opt);
    for (size_t i = 0; i < kvStrs.size(); i++) {
        const auto &kvStr = kvStrs[i];
        auto kvToken = StringTokenizer::constTokenize(kvStr, SEPARATOR_KV, opt);
        if (kvToken.size() != 2) {
            AUTIL_LOG(WARN, "invalid kv string format [%s], item value: %s.",
                      kvStr.toString().c_str(), itemValue.toString().c_str());
            return false;
        }
        keys.emplace_back(kvToken[0]);
        const auto &valueStr = kvToken[1];
        int64_t value = StringUtil::fromString<int64_t>(valueStr.toString());
        values.push_back(value);
    }
    return true;
}

}
