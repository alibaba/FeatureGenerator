#include "fg_lite/feature/UserMatchInfo.h"
#include "autil/StringTokenizer.h"
#include "autil/StringUtil.h"

using namespace std;
using namespace autil;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, UserMatchInfo);

static const string USREINFO_BETWEEN_CATEGORY_SEPARATOR = "|";
static const string USERINFO_CATEGORY_KV_SEPARATOR = "^";
static const string USERINFO_BETWEEN_KV_SEPARATOR = ",";
static const string USERINFO_KV_SEPARATOR = ":";

UserMatchInfo::UserMatchInfo() {
}

UserMatchInfo::~UserMatchInfo()  = default;

bool UserMatchInfo::parseUserInfo(const ConstString &userInfo) {
    _categoryMap.clear();
    StringTokenizer::Option opt = StringTokenizer::TOKEN_IGNORE_EMPTY | StringTokenizer::TOKEN_TRIM;
    auto categories = StringTokenizer::constTokenize(userInfo,
            USREINFO_BETWEEN_CATEGORY_SEPARATOR, opt);
    for (size_t i = 0; i < categories.size(); i++) {
        const ConstString &category = categories[i];
        auto categoryKvToken = StringTokenizer::constTokenize(category,
                USERINFO_CATEGORY_KV_SEPARATOR, opt);
        if (categoryKvToken.size() != 2) {
            AUTIL_LOG(DEBUG, "category format is invalid.category:[%s]", category.c_str());
            return false;
        }
        const auto &categoryKey = categoryKvToken[0];
        const auto &categoryValue = categoryKvToken[1];
        auto kvs = StringTokenizer::constTokenize(categoryValue,
                USERINFO_BETWEEN_KV_SEPARATOR, opt);
        for (size_t j = 0; j < kvs.size(); j++) {
            const auto &kv = kvs[j];
            auto kvToken = StringTokenizer::constTokenize(kv, USERINFO_KV_SEPARATOR, opt);
            if (kvToken.size() != 2) {
                AUTIL_LOG(WARN, "kv format is invalid. kv:[%s]", kv.c_str());
                return false;
            }
            const auto &key = kvToken[0];
            const auto &value = kvToken[1];
            _categoryMap[categoryKey][key] = value;
        }
    }
    return true;
}

bool UserMatchInfo::match(const ConstString &category, const ConstString &itemValue,
                          vector<MatchResultUnit> &matchedResult) const {
    if (ConstString(MATCH_WILDCARD_STRING) == category) {
        CategoryMap::const_iterator iter = _categoryMap.begin();
        for (; iter != _categoryMap.end(); iter++) {
            matchOneCategory(iter->first, iter->second, itemValue, matchedResult);
        }
    } else {
        CategoryMap::const_iterator iter = _categoryMap.find(ConstString(category));
        if (iter == _categoryMap.end()) {
            AUTIL_LOG(DEBUG, "can not find category [%s]", category.c_str());
            return false;
        }
        matchOneCategory(iter->first, iter->second, itemValue, matchedResult);
    }
    return !matchedResult.empty();
}

void UserMatchInfo::matchOneCategory(const ConstString &category,
                                     const KeyValueMap &kvMap,
                                     const ConstString &itemValue,
                                     vector<MatchResultUnit> &matchedResult) const
{
    if (ConstString(MATCH_WILDCARD_STRING) == itemValue) {
        KeyValueMap::const_iterator kvIter = kvMap.begin();
        for (; kvIter != kvMap.end(); kvIter++) {
            matchedResult.push_back(MatchResultUnit(category, kvIter->first, kvIter->second));
        }
    } else {
        KeyValueMap::const_iterator kvIter = kvMap.find(ConstString(itemValue));
        if (kvIter == kvMap.end()) {
            return;
        }
        matchedResult.push_back(MatchResultUnit(category, kvIter->first, kvIter->second));
    }
}

bool UserMatchInfo::match(const ConstString &category,
                          const ConstString &itemValue,
                          string &matchedValue) const
{
    CategoryMap::const_iterator iter = _categoryMap.begin();
    if (category != ConstString(MATCH_WILDCARD_STRING)) {
        iter = _categoryMap.find(category);
    }
    if (iter == _categoryMap.end()) {
        return false;
    }
    const KeyValueMap &kvMap = iter->second;
    KeyValueMap::const_iterator kvIter = kvMap.find(itemValue);
    if (kvIter == kvMap.end()) {
        return false;
    }
    matchedValue = kvIter->second.toString();
    return true;
}

bool UserMatchInfo::matchCategoryForKeys(
        const ConstString &category,
        vector<ConstString > &keys) const
{
    CategoryMap::const_iterator iter = _categoryMap.find(category);
    if (iter == _categoryMap.end()) {
        return false;
    }
    const KeyValueMap &kvMap = iter->second;
    KeyValueMap::const_iterator kvIter = kvMap.begin();
    for (; kvIter != kvMap.end(); kvIter++) {
        keys.push_back(kvIter->first);
    }
    return true;
}

bool UserMatchInfo::batchMatchForValues(const ConstString &category,
                                        const vector<ConstString> &keys,
                                        vector<int64_t> &values,
                                        int32_t &matchedCount) const {
    CategoryMap::const_iterator iter = _categoryMap.find(category);
    if (iter == _categoryMap.end()) {
        return false;
    }
    const KeyValueMap &kvMap = iter->second;
    for (size_t i = 0; i < keys.size(); i++) {
        KeyValueMap::const_iterator kvIter = kvMap.find(keys[i]);
        if (kvIter == kvMap.end()) {
            values.push_back(0);
        } else {
            const ConstString &valueStr = kvIter->second;
            int64_t value = StringUtil::fromString<int64_t>(valueStr.toString());
            values.push_back(value);
            matchedCount++;
        }
    }
    return true;
}

void UserMatchInfo::clear() {
    _categoryMap.clear();
}

bool UserMatchInfo::empty() const {
    return _categoryMap.empty();
}

}
