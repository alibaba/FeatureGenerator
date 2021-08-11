#ifndef ISEARCH_FG_LITE_USERMATCHINFO_H
#define ISEARCH_FG_LITE_USERMATCHINFO_H

#include "autil/Log.h"
#include "autil/ConstString.h"
#include "autil/MurmurHash.h"
#include <unordered_map>
#include <vector>

namespace fg_lite {
static const std::string MATCH_WILDCARD_STRING = "ALL";

struct ConstStringHasher {
    size_t operator()(const autil::ConstString &key) const {
        return autil::MurmurHash::MurmurHash64A(key.data(), key.size(), 0);
    }
};
typedef std::unordered_map<autil::ConstString, autil::ConstString, ConstStringHasher> KeyValueMap;
typedef std::unordered_map<autil::ConstString, KeyValueMap, ConstStringHasher> CategoryMap;

struct MatchResultUnit {
    autil::ConstString _category;
    autil::ConstString _item;
    autil::ConstString _value;
    MatchResultUnit(const autil::ConstString &category,
                    const autil::ConstString &item,
                    const autil::ConstString &value)
        : _category(category)
        , _item(item)
        , _value(value)
    {}
};

class UserMatchInfo
{
public:
    UserMatchInfo();
    ~UserMatchInfo();
private:
    UserMatchInfo(const UserMatchInfo &);
    UserMatchInfo& operator=(const UserMatchInfo &);
public:
    bool parseUserInfo(const autil::ConstString &userInfo);
    bool match(const autil::ConstString &category,
               const autil::ConstString &itemValue,
               std::vector<MatchResultUnit> &matchedResult) const ;
    bool match(const autil::ConstString &category,
               const autil::ConstString &itemValue,
               std::string &matchedValue) const ;
    bool matchCategoryForKeys(const autil::ConstString &category,
                              std::vector<autil::ConstString> &keys) const;
    bool batchMatchForValues(const autil::ConstString &category,
                             const std::vector<autil::ConstString> &keys,
                             std::vector<int64_t> &values,
                             int32_t &matchedCount) const;
    void clear();
    bool empty() const;
public:
    bool parseUserInfo(const std::string &userInfo) {
        return parseUserInfo(autil::ConstString(userInfo));
    }

    bool match(const std::string &category,
               const std::string &itemValue,
               std::vector<MatchResultUnit> &matchedResult) const
    {
        return match(autil::ConstString(category),
                     autil::ConstString(itemValue),
                     matchedResult);
    }
    bool match(const std::string &category,
               const std::string &itemValue,
               std::string &matchedValue) const
    {
        return match(autil::ConstString(category),
                     autil::ConstString(itemValue),
                     matchedValue);
    }
private:
    void matchOneCategory(const autil::ConstString &category,
                          const KeyValueMap &kvMap,
                          const autil::ConstString &itemValue,
                          std::vector<MatchResultUnit> &matchedResult) const;

private:
    CategoryMap _categoryMap;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_USERMATCHINFO_H
