
#include "fg_lite/feature/MatchFeatureFunction.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace std;
using namespace autil;
using namespace testing;

namespace fg_lite {

class MatchFeatureFunctionTest : public FeatureFunctionTestBase {
protected:
    FeatureFunction *createFeatureFunction(
            MatchFunction *matcher,
            bool wildCardCategory,
            bool wildCardItem)
    {
        return new MatchFeatureFunction(matcher, wildCardCategory, wildCardItem);
    }

    void genMatchFeature(const vector<FeatureInput*> &inputs,
                         MatchFunction *matcher,
                         bool wildCardCategory,
                         bool wildCardItem=false)
    {
        genFeatures(inputs, [this, matcher, wildCardCategory, wildCardItem]() {
                    return createFeatureFunction(matcher, wildCardCategory, wildCardItem);
                });
        for (auto input : inputs) {
            delete input;
        }
    }

    template <typename T>
    void genDenseInputs(const vector<string> &userValues,
                        const vector<T> &itemValues,
                        const vector<T> &categoryValues,
                        MatchFunction *matcher,
                        bool wildCardCategory,
                        bool wildCardItem=false,
                        bool userMultiValue=false)
    {
        // keep vec alive for multi value storage
        vector<vector<string>> vec;
        vector<FeatureInput*> inputs;
        if (userMultiValue) {
            for (auto v : userValues) {
                vec.push_back(vector<string>{v});
            }
            inputs.push_back(genMultiValueInput(genMultiStringValues(vec)));
        } else {
            inputs.push_back(genDenseInput<string>(userValues));
        }
        inputs.push_back(genDenseInput<T>(itemValues));
        if (!wildCardCategory) {
            inputs.push_back(genDenseInput<T>(categoryValues));
        }
        genMatchFeature(inputs, matcher, wildCardCategory, wildCardItem);
    }

};

TEST_F(MatchFeatureFunctionTest, testOneDocNotMatch) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", false, true, true);
    genDenseInputs<int64_t>(vector<string>{"50011740^107287172:abc,36806676:abc,122572685:abc|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19"},
                            vector<int64_t>{107287172, 30068, 1},
                            vector<int64_t>{50011740, 50006842, 0},
                            matcher,
                            false
                            );
    MultiDenseFeatures *typedFeatures = dynamic_cast<MultiDenseFeatures*>(_features.get());
    ASSERT_TRUE(typedFeatures != nullptr);
    vector<float> expectValue = {0.0f, 19.0f, 0.0f};
    EXPECT_EQ(typedFeatures->_featureValues, expectValue);
}


TEST_F(MatchFeatureFunctionTest, testEvaluate) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19"},
                                    vector<int64_t>{30068, 20, 30},
                                    vector<int64_t>{50006842, 2345, 3456},
                                    matcher,
                                    false);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateMultiValue1) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19"},
                            vector<int64_t>{30068, 20, 30},
                            vector<int64_t>{50006842, 2345, 3456},
                            matcher,
                            false, false, true);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateMultiValue2) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    vector<FeatureInput*> inputs;
    inputs.push_back(genDenseInput<MultiChar>(genMultiCharValues({"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19"})));
    inputs.push_back(genDenseInput(vector<int64_t>{30068, 20, 30}));
    inputs.push_back(genDenseInput(vector<int64_t>{50006842, 2345, 3456}));
    genMatchFeature(inputs, matcher, false, false);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateBatch) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19",
                    "ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19",
                    "ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:12"},
                            vector<int64_t>{30068, 20, 30068},
                            vector<int64_t>{50006842, 2345, 50006842},
                            matcher,
                            false);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19", "brand_hit_50006842_30068_12"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateBatchItemCateNotEq) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19",
                    "ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19",
                    "ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:12"},
                            vector<int64_t>{30068, 20, 30068},
                            vector<int64_t>{50006842},
                            matcher,
                            false);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19", "brand_hit_50006842_30068_12"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateBatchUserMultiValue) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19",
                    "ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19",
                    "ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:12"},
                            vector<int64_t>{30068, 20, 30068},
                            vector<int64_t>{50006842, 2345, 50006842},
                            matcher,
                            false, false, true);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19", "brand_hit_50006842_30068_12"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateWithMultiValue) {
    vector<vector<int64_t>> itemValues{{30068, 29889},{},{20,30}};
    vector<vector<int64_t>> categoryValues{{50006842, 22},{23,33},{34,44}};
    vector<string> userValues{"22^30068:11,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:12,30068:19"};

    FeatureInput *userInput = genDenseInput<string>(userValues);
    vector<autil::MultiValueType<int64_t>> multiItem = genMultiValues(itemValues);
    FeatureInput *itemInput = genMultiValueInput<int64_t>(multiItem);
    vector<autil::MultiValueType<int64_t>> multiCategory = genMultiValues(categoryValues);
    FeatureInput *categoryInput = genMultiValueInput<int64_t>(multiCategory);
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);

    vector<FeatureInput*> inputs;
    inputs.push_back(userInput);
    inputs.push_back(itemInput);
    inputs.push_back(categoryInput);
    genMatchFeature(inputs, matcher, false);
    checkMultiSparseFeatures(vector<string>{"brand_hit_50006842_30068_19",
                    "brand_hit_50006842_29889_12",
                    "brand_hit_22_30068_11"},
                             vector<size_t>{0,3,3});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateWithMultiValueWithWeighting) {
    vector<vector<int64_t>> itemValues{{30068, 29889},{},{20,30}};
    vector<vector<int64_t>> categoryValues{{50006842, 22},{23,33},{34,44}};
    vector<string> userValues{"22^30068:11,36806676:0.3,122572685:5|"
            "50006842^16788816:0.1,10122:0.2,29889:12,30068:19"};

    FeatureInput *userInput = genDenseInput<string>(userValues);
    vector<autil::MultiValueType<int64_t>> multiItem = genMultiValues(itemValues);
    FeatureInput *itemInput = genMultiValueInput<int64_t>(multiItem);
    vector<autil::MultiValueType<int64_t>> multiCategory = genMultiValues(categoryValues);
    FeatureInput *categoryInput = genMultiValueInput<int64_t>(multiCategory);
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true,
            true, true, true);

    vector<FeatureInput*> inputs;
    inputs.push_back(userInput);
    inputs.push_back(itemInput);
    inputs.push_back(categoryInput);
    genMatchFeature(inputs, matcher, false);

    checkMultiSparseWeightingFeatures(
            vector<string>({"brand_hit_50006842_30068", "brand_hit_50006842_29889", "brand_hit_22_30068"}),
            vector<double>({19, 12, 11}),
            vector<size_t>{0,3,3});
}

TEST_F(MatchFeatureFunctionTest, testEvaluateWithNULLCategory) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,30068:0.3,122572685:5"},
                                    vector<int64_t>{30068, 20, 30},
                                    vector<int64_t>{},
                                    matcher,
                                    true);
    checkMultiSparseFeatures(vector<string>{"brand_hit_ALL_30068_0.3"},
                             vector<size_t>{0,1,1});
}

TEST_F(MatchFeatureFunctionTest, testItemNotEqualCategory) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,30068:0.3,122572685:5"},
                            vector<int64_t>{30068, 20, 30},
                            vector<int64_t>{},
                            matcher,
                            false);
    MultiSparseFeatures *typedFeatures = dynamic_cast<MultiSparseFeatures*>(_features.get());
    ASSERT_TRUE(typedFeatures == nullptr);
}

TEST_F(MatchFeatureFunctionTest, testUserNull) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{},
                            vector<int64_t>{30068, 20, 30},
                            vector<int64_t>{50006842, 2345, 3456},
                            matcher,
                            false);
    checkMultiSparseFeatures(vector<string>{},
                             vector<size_t>{0, 0, 0});
}

TEST_F(MatchFeatureFunctionTest, testCheckInput) {
    vector<FeatureInput*> inputs;
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genMatchFeature(inputs, matcher, false);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(MatchFeatureFunctionTest, testUserInvalid) {
    vector<FeatureInput*> inputs;
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    FeatureInput *userInput = genDenseInput<string>(vector<string>{"abc","bca"}, 2, 1);
    inputs.push_back(userInput);
    FeatureInput *itemInput = genDenseInput<int64_t>(vector<int64_t>{1,2,3});
    inputs.push_back(itemInput);
    genMatchFeature(inputs, matcher, true);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(MatchFeatureFunctionTest, testItemInvalid) {
    MatchFunction* matcher = MatchFunction::create("hit", "brand_hit", "", true, true, true);
    genDenseInputs<int64_t>(vector<string>{"ALL^107287172:0.2,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:0.3,30068:19"},
                                    vector<int64_t>{30068, 20, 30},
                                    vector<int64_t>{},
                                    matcher,
                                    false);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(MatchFeatureFunctionTest, testItemAll) {
    vector<vector<int64_t>> categoryValues{{50006842, 22},{23,33},{34,44}};
    vector<string> userValues{"22^30068:11,36806676:0.3,122572685:5|50006842^16788816:0.1,10122:0.2,29889:12,30068:19"};

    FeatureInput *userInput = genDenseInput<string>(userValues);
    vector<autil::MultiValueType<int64_t>> multiCategory = genMultiValues(categoryValues);
    FeatureInput *categoryInput = genMultiValueInput<int64_t>(multiCategory);
    MatchFunction* matcher = MatchFunction::create("multihit", "multi_hit", "", true, true, true);

    vector<FeatureInput*> inputs;
    inputs.push_back(userInput);
    inputs.push_back(categoryInput);
    genMatchFeature(inputs, matcher, false, true);
    checkMultiSparseFeatures(vector<string>{
                "multi_hit_50006842_29889_12",
                    "multi_hit_50006842_10122_0.2",
                    "multi_hit_50006842_30068_19",
                    "multi_hit_50006842_16788816_0.1",
                    "multi_hit_22_122572685_5",
                    "multi_hit_22_36806676_0.3",
                    "multi_hit_22_30068_11"},
                             vector<size_t>{0,7,7});
}

}
