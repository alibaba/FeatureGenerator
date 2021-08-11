#include "gtest/gtest.h"
#include "gmock/gmock.h"
#define private public
#include "fg_lite/feature/UserMatchInfo.h"
#include "autil/ConstString.h"

using namespace std;
using namespace autil;
using namespace testing;

namespace fg_lite {

class UserMatchInfoTest : public ::testing::Test {
protected:
    bool isExist(const vector<MatchResultUnit> &matchedResult, const string &category, 
                 const string &item, const string &value);
};

bool UserMatchInfoTest::isExist(const vector<MatchResultUnit> &matchedResult, 
                                const string &category,
                                const string &item, 
                                const string & value) {
    for (size_t i = 0; i < matchedResult.size(); i++) {
        if (matchedResult[i]._category.toString() == category &&
            matchedResult[i]._item.toString() == item &&
            matchedResult[i]._value.toString() == value)
        {
            return true;
        }
    }
    return false;
}

static bool checkEq(const string &expected,
                    CategoryMap &categoryMap,
                    const string &category,
                    const string &itemValue)
{
    return categoryMap[ConstString(category)][ConstString(itemValue)] == ConstString(expected);
}

TEST_F(UserMatchInfoTest, testSimple) {
    UserMatchInfo userMatchInfo;
    string userInfo = "1234^10:0.1,20:0.2,30:0.3 | 2345^10:1,20:2,30:3 | 3456^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo);
    CategoryMap categoryMap = userMatchInfo._categoryMap;
    ASSERT_EQ((size_t)3, categoryMap.size());
    ASSERT_EQ((size_t)3, categoryMap[ConstString("1234")].size());
    ASSERT_TRUE(checkEq("0.1", categoryMap, "1234", "10"));
    ASSERT_TRUE(checkEq("0.2", categoryMap, "1234", "20"));
    ASSERT_TRUE(checkEq("0.3", categoryMap, "1234", "30"));
    ASSERT_TRUE(checkEq("1", categoryMap, "2345", "10"));
    ASSERT_TRUE(checkEq("2", categoryMap, "2345","20"));
    ASSERT_TRUE(checkEq("3", categoryMap,"2345","30"));
    ASSERT_TRUE(checkEq("11", categoryMap,"3456","10"));
    ASSERT_TRUE(checkEq("21", categoryMap,"3456","20"));
    ASSERT_TRUE(checkEq("31", categoryMap,"3456","30"));

    string userInfo2 = "12345^10:0.1,20:0.2,30:0.3 | 23456^10:1,20:2,30:3 | 34567^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo2);
    categoryMap.clear();
    categoryMap = userMatchInfo._categoryMap;
    ASSERT_EQ((size_t)3, categoryMap.size());
    ASSERT_EQ((size_t)0, categoryMap[ConstString("1234")].size());
    ASSERT_EQ((size_t)3, categoryMap[ConstString("12345")].size());

    userMatchInfo.clear();
    categoryMap.clear();
    categoryMap = userMatchInfo._categoryMap;
    ASSERT_EQ((size_t)0, categoryMap.size());
}

TEST_F(UserMatchInfoTest, testMatch) {
    UserMatchInfo userMatchInfo;
    string userInfo = "1234^10:0.1,20:0.2,30:0.3 | 2345^10:1,20:2,30:3 | 3456^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo);
    string matchedValue;
    ASSERT_FALSE(userMatchInfo.match("12345", "10", matchedValue));
    ASSERT_FALSE(userMatchInfo.match("1234", "11", matchedValue));
    ASSERT_TRUE(userMatchInfo.match("1234", "10", matchedValue));
    EXPECT_EQ("0.1", matchedValue);
}

TEST_F(UserMatchInfoTest, testMatch2) {
    UserMatchInfo userMatchInfo;
    string userInfo = "1234^10:0.1,20:0.2,30:0.3 | 2345^10:1,20:2,30:3 | 3456^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo);
    vector<MatchResultUnit> matchedResult;
    ASSERT_FALSE(userMatchInfo.match("ALL", "80", matchedResult));
    ASSERT_TRUE(userMatchInfo.match("ALL", "10", matchedResult));
    EXPECT_EQ((size_t)3, matchedResult.size());
    ASSERT_TRUE(isExist(matchedResult, "1234", "10", "0.1"));
    ASSERT_TRUE(isExist(matchedResult, "2345", "10", "1"));
    ASSERT_TRUE(isExist(matchedResult, "3456", "10", "11"));
}

TEST_F(UserMatchInfoTest, testMatch3) {
    UserMatchInfo userMatchInfo;
    string userInfo = "1234^10:0.1,20:0.2,30:0.3 | 2345^10:1,20:2,30:3 | 3456^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo);
    vector<MatchResultUnit> matchedResult;
    ASSERT_FALSE(userMatchInfo.match("0000", "ALL", matchedResult));
    EXPECT_EQ((size_t)0, matchedResult.size());
    ASSERT_TRUE(userMatchInfo.match("1234", "ALL", matchedResult));
    EXPECT_EQ((size_t)3, matchedResult.size());
    ASSERT_TRUE(isExist(matchedResult, "1234", "10", "0.1"));
    ASSERT_TRUE(isExist(matchedResult, "1234", "20", "0.2"));
    ASSERT_TRUE(isExist(matchedResult, "1234", "30", "0.3"));
}

TEST_F(UserMatchInfoTest, testMatchCategoryForKeys) {
    UserMatchInfo userMatchInfo;
    string userInfo = "1234^10:0.1,20:0.2,30:0.3 | 2345^10:1,20:2,30:3 | 3456^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo);
    vector<ConstString> keys;
    ASSERT_FALSE(userMatchInfo.matchCategoryForKeys(ConstString("12345"), keys));
    ASSERT_TRUE(userMatchInfo.matchCategoryForKeys(ConstString("1234"), keys));
    EXPECT_EQ((size_t)3, keys.size());
    EXPECT_THAT(keys, UnorderedElementsAre(
                    ConstString("10"), ConstString("20"), ConstString("30")));
}

TEST_F(UserMatchInfoTest, testBatchMatchForValues) {
    UserMatchInfo userMatchInfo;
    string userInfo = "1234^10:0.1,20:0.2,30:0.3 | 2345^10:1,20:2,30:3 | 3456^10:11,20:21,30:31";
    userMatchInfo.parseUserInfo(userInfo);
    vector<ConstString> keys;
    keys.push_back(ConstString("10"));
    keys.push_back(ConstString("20"));
    keys.push_back(ConstString("40"));
    vector<int64_t> values;
    int32_t matchedCount = 0;
    ASSERT_FALSE(userMatchInfo.batchMatchForValues(ConstString("12345"),
                    keys, values, matchedCount));
    ASSERT_TRUE(userMatchInfo.batchMatchForValues(ConstString("3456"),
                    keys, values, matchedCount));
    ASSERT_EQ(2, matchedCount);
    EXPECT_EQ((size_t)3, values.size());
    EXPECT_THAT(values, ElementsAre((int64_t)11,
                                    (int64_t)21,
                                    (int64_t)0));

    values.clear();
    matchedCount = 0;
    ASSERT_TRUE(userMatchInfo.batchMatchForValues(ConstString("1234")
                    , keys, values, matchedCount));
    ASSERT_EQ(2, matchedCount);
    EXPECT_EQ((size_t)3, values.size());
    EXPECT_THAT(values, ElementsAre((int64_t)0,
                                  (int64_t)0,
                                  (int64_t)0));
}

}
