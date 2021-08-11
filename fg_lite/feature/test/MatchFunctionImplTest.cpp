#include "gtest/gtest.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "fg_lite/feature/MatchFunctionImpl.h"

using namespace std;
using namespace autil;
using namespace testing;

namespace fg_lite {

class MatchFunctionImplTest : public ::testing::Test {
public:
    MultiSparseFeatures *genSparseFeatures(MatchFunction *matcher, const string &userInfo,
                             const vector<string> &categorys, const vector<string> &items)
    {
        UserMatchInfo userMatchInfo;
        userMatchInfo.parseUserInfo(ConstString(userInfo));
        MultiSparseFeatures *features = new MultiSparseFeatures(1);
        features->beginDocument();

        for (size_t i = 0; i < categorys.size(); i++) {
            for (size_t j = 0; j < items.size(); j++) {
                matcher->matchOneFeature(ConstString(categorys[i]), ConstString(items[j]), userMatchInfo, features);
            }
        }
        return features;
    }
    MultiDenseFeatures *genDenseFeatures(MatchFunction *matcher, const string &userInfo,
                             const vector<string> &categorys, const vector<string> &items)
    {
        UserMatchInfo userMatchInfo;
        userMatchInfo.parseUserInfo(ConstString(userInfo));
        MultiDenseFeatures *features = new MultiDenseFeatures("fake", 1);
        features->beginDocument();

        for (size_t i = 0; i < categorys.size(); i++) {
            for (size_t j = 0; j < items.size(); j++) {
                matcher->matchOneFeature(ConstString(categorys[i]), ConstString(items[j]), userMatchInfo, features);
            }
        }
        return features;
    }
};

TEST_F(MatchFunctionImplTest, testHitMatchNotNeedDiscrete) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("hit", "brand_hit", "", false, true, true));
    string userInfo = "50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";
    vector<string> categorys = {"500068421", "50011740", "50006842"};
    vector<string> items = {"107287172", "30068", "29889"};

    unique_ptr<MultiDenseFeatures> features(
            genDenseFeatures(matcher.get(), userInfo, categorys, items));

    vector<float> expectValue = {0.0f, 19.0f, 0.3f};
    EXPECT_EQ(features->_featureValues, expectValue);
}

TEST_F(MatchFunctionImplTest, testHitMatchNeedDiscrete) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("hit", "brand_hit", "", true, true, true));
    string userInfo = "50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";
    vector<string> categorys = {"50006842"};
    vector<string> items = {"30068", "29889"};

    unique_ptr<MultiSparseFeatures> features(
            genSparseFeatures(matcher.get(), userInfo, categorys, items));

    vector<ConstString> expectNames = {
        ConstString("brand_hit_50006842_30068_19"),
        ConstString("brand_hit_50006842_29889_0.3")
    };
    EXPECT_EQ(features->_featureNames, expectNames);
}

TEST_F(MatchFunctionImplTest, testHitMatchWithNormalizer) {
    // equivalent to featureValue / 2
    unique_ptr<MatchFunction> matcher(MatchFunction::create("hit", "brand_hit", "method=minmax,min=0.0,max=2.0", false, true, true));
    string userInfo = "50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";
    vector<string> categorys = {"500068421", "50011740", "50006842"};
    vector<string> items = {"107287172", "30068", "29889"};

    unique_ptr<MultiDenseFeatures> features(
            genDenseFeatures(matcher.get(), userInfo, categorys, items));

    vector<float> expectValues = {0.0f, 9.5f, 0.15f};
    EXPECT_EQ(features->_featureValues, expectValues);
}

TEST_F(MatchFunctionImplTest, testHitMatchNeedDiscreteAll) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("hit", "brand_hit", "", true, true, true));
    string userInfo = "ALL^16788816:0.1,10122:0.2,29889:0.3,30068:19";

    vector<string> categorys = {MATCH_WILDCARD_STRING};
    vector<string> items = {"30068", "29889"};

    unique_ptr<MultiSparseFeatures> features(
            genSparseFeatures(matcher.get(), userInfo, categorys, items));

    vector<ConstString> expectNames = {
        ConstString("brand_hit_ALL_30068_19"),
        ConstString("brand_hit_ALL_29889_0.3")
    };
    EXPECT_EQ(features->_featureNames, expectNames);
}

TEST_F(MatchFunctionImplTest, testMultiHitMatchNotNeedDiscrete) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("multihit", "brand_hit", "", false, true, true));
    string userInfo = "50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";

    vector<string> categorys = {"500068421", "50011740", "50006842"};
    vector<string> items = {"107287172", "30068", "29889"};

    unique_ptr<MultiDenseFeatures> features(
            genDenseFeatures(matcher.get(), userInfo, categorys, items));

    EXPECT_THAT(features->_featureValues, ElementsAre(0.0f, 19.0f, 0.3f));
}

TEST_F(MatchFunctionImplTest, testMultiHitMatchNotNeedDiscreteAll) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("multihit", "brand_hit", "", false, true, true));
    string userInfo = "50011740^107287172:1,36806676:2,122572685:3|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";
    vector<string> categorys = {MATCH_WILDCARD_STRING};
    vector<string> items = {MATCH_WILDCARD_STRING};

    unique_ptr<MultiDenseFeatures> features(
            genDenseFeatures(matcher.get(), userInfo, categorys, items));

    EXPECT_THAT(features->_featureValues, UnorderedElementsAre(
                    0.3f, 0.2f, 19.0f, 0.1f, 3.0f, 2.0f, 1.0f));
}

TEST_F(MatchFunctionImplTest, testMultiHitMatchNeedDiscreteAll) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("multihit", "brand_hit", "", true, true, true));
    string userInfo = "50011740^30068:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";
    {
        vector<string> categorys = {MATCH_WILDCARD_STRING};
        vector<string> items = {"30068"};

        unique_ptr<MultiSparseFeatures> features(
                genSparseFeatures(matcher.get(), userInfo, categorys, items));

        vector<ConstString> expectNames = {
            ConstString("brand_hit_50006842_30068_19"),
            ConstString("brand_hit_50011740_30068_abc"),
        };
        EXPECT_EQ(features->_featureNames, expectNames);
    }
    {
        vector<string> categorys = {"50011740"};
        vector<string> items = {MATCH_WILDCARD_STRING};

        unique_ptr<MultiSparseFeatures> features(
                genSparseFeatures(matcher.get(), userInfo, categorys, items));

        EXPECT_THAT(features->_featureNames, UnorderedElementsAre(
                        ConstString("brand_hit_50011740_122572685_abc"),
                        ConstString("brand_hit_50011740_36806676_abc"),
                        ConstString("brand_hit_50011740_30068_abc")));
    }
}

TEST_F(MatchFunctionImplTest, testMultiHitMatchNeedDiscreteShowCategory) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("multihit", "brand_hit", "", true, true, false));
    string userInfo = "50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";

    vector<string> categorys = {"50006842", "50011740"};
    vector<string> items = {"30068", "36806676"};

    unique_ptr<MultiSparseFeatures> features(
            genSparseFeatures(matcher.get(), userInfo, categorys, items));

    vector<ConstString> expectNames = {
        ConstString("brand_hit_50006842_19"),
        ConstString("brand_hit_50011740_abc")
    };
    EXPECT_EQ(features->_featureNames, expectNames);

}

TEST_F(MatchFunctionImplTest, testMultiHitMatchNeedDiscreteShowItem) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("multihit", "brand_hit", "", true, false, true));
    string userInfo = "50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19";
    vector<string> categorys = {"50006842", "50011740"};
    vector<string> items = {"30068", "36806676"};

    unique_ptr<MultiSparseFeatures> features(
            genSparseFeatures(matcher.get(), userInfo, categorys, items));

    vector<ConstString> expectNames = {
        ConstString("brand_hit_30068_19"),
        ConstString("brand_hit_36806676_abc")
    };
    EXPECT_EQ(features->_featureNames, expectNames);
}


TEST_F(MatchFunctionImplTest, testCrossMatch) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("cross", "brand_cross", "", true, true, true));
    string userInfo = "50011740^107287172:0.2,36806676:0.3|50006842^10122:0.2,29889:0.3";

    vector<string> categorys = {"123456", "50006842"};
    vector<string> items = {"29889", "10122"};
    
    unique_ptr<MultiSparseFeatures> features(
            genSparseFeatures(matcher.get(), userInfo, categorys, items));

    vector<ConstString> expectNames = {
            ConstString("brand_cross_50006842_29889_29889"),
            ConstString("brand_cross_50006842_10122_29889"),
            ConstString("brand_cross_50006842_29889_10122"),
            ConstString("brand_cross_50006842_10122_10122")
        };
    EXPECT_EQ(features->_featureNames, expectNames);
}

TEST_F(MatchFunctionImplTest, testCosMatchNeedDiscrete) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("cos", "brand_matched_cos", "", true, true, true));
    ASSERT_FALSE(matcher->needWeighting());

    string userInfo = "50011740^107287172:39,36806676:39,122572685:39|50006842^16788816:40,10122:40,29889:20,30068:20";
    {
        vector<string> categorys = {"50006842"};
        vector<string> items = {"16788816:40,101220:40,298890:20,300680:20",
                                "16788816:40,101220:40,29889:20,300680:20"};
    
        unique_ptr<MultiSparseFeatures> features(
                genSparseFeatures(matcher.get(), userInfo, categorys, items));

        vector<ConstString> expectNames = {
            ConstString("brand_matched_cos_16"),
            ConstString("brand_matched_cos_10")
        };
        EXPECT_EQ(features->_featureNames, expectNames);
    }
    {
        vector<string> categorys = {"50006842", "500068420"};
        vector<string> items = {"16788816:40,101220:40,29889:20,300680:20:12",
                                "167888160:40,101220:40,298890:20,300680:20"};
    
        unique_ptr<MultiSparseFeatures> features(
                genSparseFeatures(matcher.get(), userInfo, categorys, items));

        vector<ConstString> expectNames = {
            ConstString("brand_matched_cos_0")
        };
        EXPECT_EQ(features->_featureNames, expectNames);
    }

}

TEST_F(MatchFunctionImplTest, testCosMatchNotNeedDiscrete) {
    unique_ptr<MatchFunction> matcher(MatchFunction::create("cos", "brand_matched_cos", "", false, true, true));
    string userInfo = "50011740^107287172:39,36806676:39,122572685:39|50006842^16788816:40,10122:40,29889:20,30068:20";
    vector<string> categorys = {"50006842"};
    vector<string> items = {"16788816:40,101220:40,298890:20,300680:20",
                            "16788816:40,101220:40,29889:20,300680:20"};

    unique_ptr<MultiDenseFeatures> features(
            genDenseFeatures(matcher.get(), userInfo, categorys, items));

    EXPECT_THAT(features->_featureValues, ElementsAre(
                    16.0f, 10.0f));
}

}
