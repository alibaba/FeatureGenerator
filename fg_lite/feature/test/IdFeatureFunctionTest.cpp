
#include "fg_lite/feature/IdFeatureFunction.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace std;
using namespace autil;
using namespace testing;

namespace fg_lite {

class IdFeatureFunctionTest : public FeatureFunctionTestBase {
protected:
    void genIdFeature(FeatureInput *input,
                      const string &prefix = "prefix_",
                      int32_t pruneTo = numeric_limits<int>::max(),
                      const vector<string> &invalidValues = {})
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input);
        genFeatures(inputs, [=]() {
                                return new IdFeatureFunction("", prefix, pruneTo, invalidValues);
                });
    }
protected:
    template <typename T>
    void testDenseInput(const vector<T> &values, const vector<string> &results) {
        unique_ptr<FeatureInput> input(genDenseInput<T>(values));
        genIdFeature(input.get());
        auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleSparseFeatures, _features.get());
        EXPECT_EQ(typedFeatures->_featureNames.size(), results.size());
        for (size_t i = 0; i < results.size(); i++) {
            EXPECT_EQ(typedFeatures->_featureNames[i], ConstString(results[i]));
        }
    }

    template <typename T>
    void testValueOffsetInput(const vector<T> &values,
                              const vector<size_t> &offset,
                              const vector<string> &results)
    {
        unique_ptr<FeatureInput> input(genValueOffsetInput<T>(values, offset));
        genIdFeature(input.get());
        MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures,
                _features.get());
        EXPECT_EQ(typedFeatures->_featureNames.size(), results.size());
        for (size_t i = 0; i < results.size(); i++) {
            EXPECT_EQ(typedFeatures->_featureNames[i], ConstString(results[i]));
        }
    }

    template <typename T>
    void testMultiValueInput(const vector<vector<T>> &values,
                             const vector<string> &results)
    {
        vector<autil::MultiValueType<T>> multiValues = genMultiValues(values);
        unique_ptr<FeatureInput> input(genMultiValueInput<T>(multiValues));
        genIdFeature(input.get());
        MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures,
                _features.get());
        EXPECT_EQ(typedFeatures->_featureNames.size(), results.size());
        for (size_t i = 0; i < results.size(); i++) {
            EXPECT_EQ(typedFeatures->_featureNames[i], ConstString(results[i]));
        }
    }

    void checkOffsets(const vector<size_t> &results) {
        MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures,
                _features.get());
        EXPECT_EQ(results.size(), typedFeatures->_offsets.size());
        for (size_t i = 0; i < results.size(); i++) {
            EXPECT_EQ(results[i], typedFeatures->_offsets[i]);
        }
    }
};

TEST_F(IdFeatureFunctionTest, testDenseInput) {
    testDenseInput(vector<int8_t>{1,2,3}, vector<string>{"prefix_1", "prefix_2", "prefix_3"});
    testDenseInput(vector<int16_t>{1,2,3}, vector<string>{"prefix_1", "prefix_2", "prefix_3"});
    testDenseInput(vector<int32_t>{1,2,3}, vector<string>{"prefix_1", "prefix_2", "prefix_3"});
    testDenseInput(vector<int64_t>{1,2,3}, vector<string>{"prefix_1", "prefix_2", "prefix_3"});
    testDenseInput(vector<float>{1.1,2.4,3.9,-5.1,-9.7},
                   vector<string>{"prefix_1", "prefix_2", "prefix_4", "prefix_-5", "prefix_-10"});
    testDenseInput(vector<double>{1.1,2.2,3.8}, vector<string>{"prefix_1", "prefix_2", "prefix_4"});
    testDenseInput(vector<string>{"abc", "aaa", "ccc"},
                   vector<string>{"prefix_abc", "prefix_aaa", "prefix_ccc"});
}

TEST_F(IdFeatureFunctionTest, testNoPrefix) {
    vector<string> values{"abc"};
    unique_ptr<FeatureInput> input(genDenseInput<string>(values));
    genIdFeature(input.get(), "");
    SingleSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(SingleSparseFeatures,
            _features.get());
    EXPECT_THAT(typedFeatures->_featureNames, ElementsAre(ConstString("abc")));
}

TEST_F(IdFeatureFunctionTest, testValueOffset) {
    testValueOffsetInput(vector<int32_t>{0,1,2,5}, vector<size_t>{0,1},
                         vector<string>{"prefix_0", "prefix_1", "prefix_2", "prefix_5"});
    testValueOffsetInput(vector<string>{"abc", "aaa", "ccc"}, vector<size_t>{0,2},
                         vector<string>{"prefix_abc", "prefix_aaa", "prefix_ccc"});
}

TEST_F(IdFeatureFunctionTest, testMultiValue) {
    testMultiValueInput(vector<vector<int32_t>>{{1,2},{4,5}},
                        vector<string>{"prefix_1", "prefix_2", "prefix_4", "prefix_5"});
}

TEST_F(IdFeatureFunctionTest, testMultiChar) {
    vector<MultiChar> values = genMultiCharValues(vector<string>{"3.75", "abc", "3.75 abc"});
    testDenseInput(values,
                   vector<string>{"prefix_3.75", "prefix_abc", "prefix_3.75 abc"});
    testValueOffsetInput(values, vector<size_t>{0,1},
                         vector<string>{"prefix_3.75", "prefix_abc", "prefix_3.75 abc"});
}

TEST_F(IdFeatureFunctionTest, testMultiString) {
    vector<vector<string>> values{{"abc", "aaa"}, {"ccc"}};
    auto multiValues = genMultiStringValues(values);
    unique_ptr<FeatureInput> input(genMultiValueInput<MultiChar>(multiValues));
    genIdFeature(input.get());
    MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures,
            _features.get());
    EXPECT_EQ((size_t)3, typedFeatures->_featureNames.size());
    EXPECT_THAT(typedFeatures->_featureNames, ElementsAre(ConstString("prefix_abc"),
                    ConstString("prefix_aaa"),
                    ConstString("prefix_ccc")));
}

TEST_F(IdFeatureFunctionTest, testInvalidValue) {
    testDenseInput(vector<uint8_t>{1, 255, 2},
                   vector<string>{"prefix_1", "prefix_2"});
    checkOffsets(vector<size_t>{0,1,1});

    testValueOffsetInput(vector<uint8_t>{0,1,255,5}, vector<size_t>{0,1},
                         vector<string>{"prefix_0", "prefix_1", "prefix_5"});
}

TEST_F(IdFeatureFunctionTest, testInvalidValueConfig) {
    unique_ptr<FeatureInput> input(genDenseInput<int32_t>({1, 2, 3}));
    genIdFeature(input.get(), "", 100, {"2"});
    auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleSparseFeatures, _features.get());
    vector<string> results = {"1", "3"};
    EXPECT_EQ(typedFeatures->_featureNames.size(), results.size());
    for (size_t i = 0; i < results.size(); i++) {
        EXPECT_EQ(typedFeatures->_featureNames[i], ConstString(results[i]));
    }
    checkOffsets(vector<size_t>{0,1,1});
}

}
