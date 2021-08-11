#include "fg_lite/feature/PreclickUrbWordFeatureFunction.h"
#include <vector>
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace std;
using namespace autil;
using namespace testing;

namespace fg_lite {
class PreclickUrbWordFeatureFunctionTest : public FeatureFunctionTestBase {
protected:
    FeatureFunction *createFeatureFuntion(const string &name,
                                          const string &prefix,
                                          bool need_match = false,
                                          bool output_count = false,
                                          std::string delim_item = ";",
                                          std::string delim_kv = "",
                                          bool need_decode = true) {
        return new PreclickUrbWordFeatureFunction(name, prefix, need_decode, need_match, output_count, delim_item, delim_kv, false,
                                                  false);
    }

    void genFeature(vector<FeatureInput*> &inputs,
                    const string &name = "preclick_urb_word_feature",
                    const string &prefix = "_",
                    bool need_match = false,
                    bool output_count = false,
                    std::string delim_item = ";",
                    std::string delim_kv = "",
                    bool need_decode = true) {
        genFeatures(inputs, [this, name, prefix, need_match, output_count, delim_item, delim_kv, need_decode]() {
            return createFeatureFuntion(name, prefix, need_match, output_count, delim_item, delim_kv, need_decode);
        });
    }

    void testTopFeature(const vector<vector<std::string>> &values,
                   const vector<size_t> &offsets,
                   const vector<std::string> &results,
                   std::string delim_item = ";",
                   std::string delim_kv = "",
                   bool need_decode = true) {
        auto multiValues = genMultiStringValues(values);
        unique_ptr<FeatureInput> input(genMultiValueInput<MultiChar>(multiValues));
        
        vector<FeatureInput*> inputs;
        inputs.push_back(input.get());
        genFeature(inputs, "preclick_urb_word_feature", "_", false, false, delim_item, delim_kv, need_decode);

        MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures, _features.get());

        EXPECT_EQ(typedFeatures->count(), offsets.size());
        EXPECT_EQ(typedFeatures->_featureNames.size(), results.size());
        for (size_t i = 0; i < offsets.size(); i++) {
            EXPECT_EQ(typedFeatures->_offsets[i], offsets[i]);
        }
        for (size_t i = 0; i < results.size(); i++) {
            EXPECT_EQ(typedFeatures->_featureNames[i], ConstString(results[i]));
        }
    }
    
    void testMatchFeature(const vector<vector<std::string>> &expressionValues,
                          const vector<vector<std::string>> &matchValues,
                          const vector<size_t> &offsets,
                          const vector<std::string> &results, 
                          bool output_count,
                          std::string delim_item = ";",
                          std::string delim_kv = "",
                          bool need_decode = true) {
        auto eValues = genMultiStringValues(expressionValues);
        auto mValues = genMultiStringValues(matchValues);

        unique_ptr<FeatureInput> eInput(genMultiValueInput<MultiChar>(eValues));
        unique_ptr<FeatureInput> mInput(genMultiValueInput<MultiChar>(mValues));

        vector<FeatureInput*> inputs;
        inputs.push_back(eInput.get());
        inputs.push_back(mInput.get());
        genFeature(inputs, "preclick_urb_word_feature", "_", true, output_count, delim_item, delim_kv, need_decode);

        MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures, _features.get());

        EXPECT_EQ(typedFeatures->count(), offsets.size());
        EXPECT_EQ(typedFeatures->count(), matchValues.size());
        EXPECT_EQ(typedFeatures->_featureNames.size(), results.size());
        for (size_t i = 0; i < offsets.size(); i++) {
            EXPECT_EQ(typedFeatures->_offsets[i], offsets[i]);
        }
        for (size_t i = 0; i < results.size(); i++) {
            EXPECT_EQ(typedFeatures->_featureNames[i], ConstString(results[i]));
        }
    }
};

TEST_F(PreclickUrbWordFeatureFunctionTest, testTopMulti) {
    testTopFeature({{"YWR3b3JkMTthZHdvcmQyO2Fkd29yZDM7YWR3b3JkNA==", "YWR3b3JkMTthZHdvcmQyO2Fkd29yZDM7YWR3b3JkNA=="}, {"YWR3b3JkMTthZHdvcmQyO2Fkd29yZDM7YWR3b3JkNA=="}}, {0, 4}, {"_adword1", "_adword2", "_adword3", "_adword4", "_adword1", "_adword2", "_adword3", "_adword4"});
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testTopMultiUncoded) {
    testTopFeature({{"adword1;adword2;adword3;adword4", "adword1;adword2;adword3;adword4"}, {"adword1;adword2;adword3;adword4"}}, {0, 4}, {"_adword1", "_adword2", "_adword3", "_adword4", "_adword1", "_adword2", "_adword3", "_adword4"}, ";", "", false);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testTopOrder) {
    testTopFeature({{"NTs1OzU7NTs1", "NDs0OzQ7NA==", "MzszOzM=", "Mjsy", "MQ=="}}, {0}, {"_5", "_4", "_3", "_2", "_1"});
}
/*
TEST_F(PreclickUrbWordFeatureFunctionTest, testTopItemLimit) {
    testTopFeature({{"Njs2OzY7Njs2OzY=", "NTs1OzU7NTs1", "NDs0OzQ7NA==", "MzszOzM=", "Mjsy", "MQ==", "NTs1OzU7NTs1", "NDs0OzQ7NA==", "MzszOzM=", "Mjsy", "MQ=="}}, {0}, {"_5", "_4", "_3", "_2", "_1"});
}
*/
TEST_F(PreclickUrbWordFeatureFunctionTest, testTopDelimComma) {
    testTopFeature({{"MSwyLDMsNCw1"}}, {0}, {"_1", "_2", "_3", "_4", "_5"}, ",", "");
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testTopDelimKv) {
    testTopFeature({{"YToxO2I6MjtjOjM7ZDo0O2U6NQ=="}}, {0}, {"_1", "_2", "_3", "_4", "_5"}, ";", ":");
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testTopWordLimit) {
    testTopFeature({{"MTsyOzE7MzsxOzI7NDsxOzI7Mzs1OzE7MjszOzQ7NjsxOzI7Mzs0OzU7NzsxOzI7Mzs0OzU7Njs4OzE7MjszOzQ7NTs2Ozc7OTsxOzI7Mzs0OzU7Njs3Ozg7MTA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTI7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTM7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTQ7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTU7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTY7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTc7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTg7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTk7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MjA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}}, {0}, {"_1", "_2", "_3", "_4", "_5", "_6", "_7", "_8", "_9", "_10", "_11", "_12", "_13", "_14", "_15", "_16", "_17", "_18", "_19", "_20"});
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchMutli) {
    testMatchFeature({{"MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA=="}}, {{"MTsyOzM7NDs1"}, {"NTs2", "Njs3"}}, {0, 5}, {"_1", "_2", "_3", "_4", "_5", "_5", "_6", "_7"}, false);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchMutliUncode) {
    testMatchFeature({{"1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16;17;18;19;20"}}, {{"1;2;3;4;5"}, {"5;6", "6;7"}}, {0, 5}, {"_1", "_2", "_3", "_4", "_5", "_5", "_6", "_7"}, false, ";", "", false);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchCountMutli) {
    testMatchFeature({{"MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA=="}}, {{"MTsyOzM7NDs1"}, {"NTs2", "Njs3"}}, {0, 1}, {"_5", "_3"}, true);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchNoMatch) {
    testMatchFeature({{"MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA=="}}, {{"MjA7MjE7MjI="}, {"MjI7MjM7MjQ="}, {"MjQ7MjU7MjY="}}, {0, 1, 1}, {"_20"}, false);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchNoMatchCount) {
    testMatchFeature({{"MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA=="}}, {{"MjA7MjE7MjI="}, {"MjI7MjM7MjQ="}, {"MjQ7MjU7MjY="}}, {0, 1, 2}, {"_1", "_0", "_0"}, true);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchWordLong) {
    testMatchFeature({{"MTsyOzE7MzsxOzI7NDsxOzI7Mzs1OzE7MjszOzQ7NjsxOzI7Mzs0OzU7NzsxOzI7Mzs0OzU7Njs4OzE7MjszOzQ7NTs2Ozc7OTsxOzI7Mzs0OzU7Njs3Ozg7MTA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTI7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTM7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTQ7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTU7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTY7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTc7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTg7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTk7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MjA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}}
    , {{"MTsyOzM=", "Mzs0"}, {"OTs4Ozc=", "Mg=="}}
    , {0, 4}
    , {"_1", "_2", "_3", "_4", "_2", "_7", "_8", "_9"}
    , false);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchWordLongCount) {
    testMatchFeature({{"MTsyOzE7MzsxOzI7NDsxOzI7Mzs1OzE7MjszOzQ7NjsxOzI7Mzs0OzU7NzsxOzI7Mzs0OzU7Njs4OzE7MjszOzQ7NTs2Ozc7OTsxOzI7Mzs0OzU7Njs3Ozg7MTA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTI7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTM7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTQ7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTU7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTY7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTc7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTg7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTk7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MjA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}}
    , {{"MTsyOzM=", "Mzs0"}, {"OTs4Ozc=", "Mg=="}}
    , {0, 1}
    , {"_4", "_4"}
    , true);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchDelimComma) {
    testMatchFeature({{"MSwyLDMsNCw1"}}, {{"MSwyLDM="}, {"Myw0LDU="}}, {0, 3}, {"_1", "_2", "_3", "_3", "_4", "_5"}, false, ",", "");
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchDelimCommaCount) {
    testMatchFeature({{"MSwyLDMsNCw1"}}, {{"MSwyLDM="}, {"Myw0LDU="}}, {0, 1}, {"_3", "_3"}, true, ",", "");
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchDelimKv) {
    testMatchFeature({{"YToxO2I6MjtjOjM7ZDo0O2U6NQ=="}}, {{"YToxO2I6MjtjOjM="}, {"YzozO2Q6NDtlOjU="}}, {0, 3}, {"_1", "_2", "_3", "_3", "_4", "_5"}, false, ";", ":");
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchDelimKvCount) {
    testMatchFeature({{"YToxO2I6MjtjOjM7ZDo0O2U6NQ=="}}, {{"YToxO2I6MjtjOjM="}, {"YzozO2Q6NDtlOjU="}}, {0, 1}, {"_3", "_3"}, true, ";", ":");
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchWordLimit) {
    testMatchFeature({{"MTsyOzE7MzsxOzI7NDsxOzI7Mzs1OzE7MjszOzQ7NjsxOzI7Mzs0OzU7NzsxOzI7Mzs0OzU7Njs4OzE7MjszOzQ7NTs2Ozc7OTsxOzI7Mzs0OzU7Njs3Ozg7MTA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTI7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTM7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTQ7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTU7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTY7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTc7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTg7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTk7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MjA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}}
    , {{"MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}, {"MjE7MTk7MjA="}}
    , {0, 20}
    , {"_1", "_2", "_3", "_4", "_5", "_6", "_7", "_8", "_9", "_10", "_11", "_12", "_13", "_14", "_15", "_16", "_17", "_18", "_19", "_20", "_19", "_20", "_21"}
    , false);
}

TEST_F(PreclickUrbWordFeatureFunctionTest, testMatchWordLimitCount) {
    testMatchFeature({{"MTsyOzE7MzsxOzI7NDsxOzI7Mzs1OzE7MjszOzQ7NjsxOzI7Mzs0OzU7NzsxOzI7Mzs0OzU7Njs4OzE7MjszOzQ7NTs2Ozc7OTsxOzI7Mzs0OzU7Njs3Ozg7MTA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTI7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTM7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTQ7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTU7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTY7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTc7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTg7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTk7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MjA7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}}
    , {{"MjE7MTsyOzM7NDs1OzY7Nzs4Ozk7MTA7MTE7MTI7MTM7MTQ7MTU7MTY7MTc7MTg7MTk7MjA="}, {"MjE7MTk7MjA="}}
    , {0, 1}
    , {"_20", "_3"}
    , true);
}

}

