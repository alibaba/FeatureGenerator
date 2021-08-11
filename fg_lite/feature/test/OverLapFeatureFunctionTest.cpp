

#include "fg_lite/feature/FeatureConfig.h"
#include "fg_lite/feature/FeatureFunctionCreator.h"
#include "fg_lite/feature/OverLapFeatureFunction.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {

class OverLapFeatureFunctionTest : public FeatureFunctionTestBase {
protected:
    FeatureFunction *createFeatureFunction(
            const string &featureName,
            OverLapFeatureConfig::OverLapType method,
            const string &separator,
            bool needDiscrete,
            bool needPrefix = false,
            bool needDense = false,
            int cutTh = -1)
    {
        return new OverLapFeatureFunction(featureName, method, separator, needDiscrete, needPrefix, needDense, cutTh);
    }

    void genOverLapFeature(vector<FeatureInput*> inputs,
                           const string &featureName = "fg",
                           OverLapFeatureConfig::OverLapType method = OverLapFeatureConfig::OT_EQUAL,
                           const string &separator = "_",
                           bool needDiscrete = true,
                           bool needPrefix = false,
                           bool needDense = false,
                           int cutTh = -1)
    {
        genFeatures(inputs, [=]() {
                    return createFeatureFunction(featureName, method, separator, needDiscrete, needPrefix, needDense, cutTh);
                });
    }
    template <typename T>
    void testSimpleDense(const vector<T> &userValues,
                         const vector<vector<T>> &itemValues,
                         OverLapFeatureConfig::OverLapType method,
                         const vector<float> &values)
    {
        unique_ptr<FeatureInput> input1(genDenseInput<T>(userValues, 1, userValues.size()));
        vector<MultiValueType<T>> multiItemValues = genMultiValues(itemValues);
        unique_ptr<FeatureInput> input2(genMultiValueInput<T>(multiItemValues));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genOverLapFeature(inputs, "fg", method, "", false);
        checkSingleDenseFeatures(values);
    }
    template <typename T>
    void testSimpleSparse(const vector<T> &userValues,
                          const vector<vector<T>> &itemValues,
                          OverLapFeatureConfig::OverLapType method,
                          const vector<std::string> &values)
    {
        unique_ptr<FeatureInput> input1(genDenseInput<T>(userValues, 1, userValues.size()));
        vector<MultiValueType<T>> multiItemValues = genMultiValues(itemValues);
        unique_ptr<FeatureInput> input2(genMultiValueInput<T>(multiItemValues));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genOverLapFeature(inputs, "fg", method, "_", true, true, true);
        checkSingleSparseFeatures(values);
    }

    template <typename T>
    void testSimpleSparse(const vector<T> &userValues,
                         const vector<vector<T>> &itemValues,
                         const OverLapFeatureConfig::OverLapType &method,
                         const vector<std::string> &values,
                         int cutTh)
    {
        unique_ptr<FeatureInput> input1(genDenseInput<T>(userValues, 1, userValues.size()));
        vector<MultiValueType<T>> multiItemValues = genMultiValues(itemValues);
        unique_ptr<FeatureInput> input2(genMultiValueInput<T>(multiItemValues));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genOverLapFeature(inputs, "fg", method, "_", true, true, true, 1);
        checkSingleSparseFeatures(values);
    }

    template <typename T>
    void testSameValueType(const vector<T> &userValues,
                           const vector<vector<T>> &itemValues,
                           OverLapFeatureConfig::OverLapType method,
                           const vector<string> &names,
                           const vector<size_t> &offsets)
    {
        unique_ptr<FeatureInput> input1(genDenseInput<T>(userValues, 1, userValues.size()));
        vector<MultiValueType<T>> multiItemValues = genMultiValues(itemValues);
        unique_ptr<FeatureInput> input2(genMultiValueInput<T>(multiItemValues));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genOverLapFeature(inputs, "fg", method);
        checkMultiSparseFeatures(names, offsets);
    }

};

TEST_F(OverLapFeatureFunctionTest, testEqual) {
    testSameValueType(vector<int32_t>{1,2,3},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                      OverLapFeatureConfig::OT_EQUAL,
                      vector<string>{"fg_0","fg_1"},
                      vector<size_t>{0,1});
}

TEST_F(OverLapFeatureFunctionTest, testContain) {
    testSameValueType(vector<int64_t>{1,2,3},
                      vector<vector<int64_t>>{{1,3,5},{1,2,3}},
                      OverLapFeatureConfig::OT_CONTAIN,
                      vector<string>{"fg_0","fg_1"},
                      vector<size_t>{0,1});
}

TEST_F(OverLapFeatureFunctionTest, testCommonWord) {
    testSameValueType(vector<int32_t>{1,2,3},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                      OverLapFeatureConfig::OT_COMMON_WORDS,
                      vector<string>{"fg_1_3", "fg_1_2_3"},
                      vector<size_t>{0,1});
}

TEST_F(OverLapFeatureFunctionTest, testCommonWordDivided) {
    testSameValueType(vector<int32_t>{1,2,3},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                      OverLapFeatureConfig::OT_COMMON_WORDS_DIVIDED,
                      vector<string>{"fg_1", "fg_3", "fg_1", "fg_2", "fg_3"},
                      vector<size_t>{0,2});
}

TEST_F(OverLapFeatureFunctionTest, testDiffWord) {
    testSameValueType(vector<int32_t>{1,2,3},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3},{2,3}},
                      OverLapFeatureConfig::OT_DIFF_WORDS,
                      vector<string>{"fg_2","fg_1"},
                      vector<size_t>{0,1,1});
}

TEST_F(OverLapFeatureFunctionTest, testDiffWordDivided) {
    testSameValueType(vector<int32_t>{1,2,3,4},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3},{2,3}},
                      OverLapFeatureConfig::OT_DIFF_WORDS_DIVIDED,
                      vector<string>{"fg_2","fg_4", "fg_4", "fg_1", "fg_4"},
                      vector<size_t>{0,2,3});
}

TEST_F(OverLapFeatureFunctionTest, testQueryRatio) {
    testSameValueType(vector<int32_t>{1,2,3},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                      OverLapFeatureConfig::OT_QUERY_RATIO,
                      vector<string>{"fg_6","fg_10"},
                      vector<size_t>{0,1});
}

TEST_F(OverLapFeatureFunctionTest, testQueryRatioDense) {
    testSimpleDense(vector<int32_t>{1,2,3},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_QUERY_RATIO,
                    {6 ,10});
}

TEST_F(OverLapFeatureFunctionTest, testTitleRatio) {
    testSameValueType(vector<int32_t>{1,2,3},
                      vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                      OverLapFeatureConfig::OT_TITLE_RATIO,
                      vector<string>{"fg_66","fg_100"},
                      vector<size_t>{0,1});
}

TEST_F(OverLapFeatureFunctionTest, testTitleRatioDense) {
    testSimpleDense(vector<int32_t>{1,2,3},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_TITLE_RATIO,
                    {66 ,100});
}

TEST_F(OverLapFeatureFunctionTest, testItemDense) {
    vector<int32_t> value1{1,2,3};
    vector<int32_t> value2{1,2,3};
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(value1, 1, 3));
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(value2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
    checkMultiSparseFeatures(vector<string>{"fg_1", "fg_2", "fg_3"}, vector<size_t>{0,1,2});
}

TEST_F(OverLapFeatureFunctionTest, testUserMultiValue) {
    vector<vector<int32_t>> userValue{{1,2,3}};
    vector<int32_t> itemValue{1,2,3};
    vector<MultiValueType<int32_t>> multiUserValues = genMultiValues(userValue);
    unique_ptr<FeatureInput> input1(genMultiValueInput<int32_t>(multiUserValues));
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(itemValue));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
    checkMultiSparseFeatures(vector<string>{"fg_1", "fg_2", "fg_3"}, vector<size_t>{0,1,2});
}

TEST_F(OverLapFeatureFunctionTest, testItemMultiValue) {
    vector<vector<int32_t>> userValue{{1,2,3}};
    vector<vector<int32_t>> itemValue{{1},{2},{3}};
    vector<MultiValueType<int32_t>> multiUserValues = genMultiValues(userValue);
    unique_ptr<FeatureInput> input1(genMultiValueInput<int32_t>(multiUserValues));
    vector<MultiValueType<int32_t>> multiItemValues = genMultiValues(itemValue);
    unique_ptr<FeatureInput> input2(genMultiValueInput<int32_t>(multiItemValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
    checkMultiSparseFeatures(vector<string>{"fg_1", "fg_2", "fg_3"}, vector<size_t>{0,1,2});
}


TEST_F(OverLapFeatureFunctionTest, testItemNotEqualUserType) {
    vector<int32_t> value1{1,2,3};
    vector<vector<double>> value2{{1.0},{2.0},{3.0}};
    vector<MultiValueType<double>> multiItemValues = genMultiValues(value2);
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(value1, 1, 3));
    unique_ptr<FeatureInput> input2(genMultiValueInput<double>(multiItemValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_EQUAL);
    checkMultiSparseFeatures(vector<string>{"fg_0", "fg_0", "fg_0"}, vector<size_t>{0,1,2});
}

TEST_F(OverLapFeatureFunctionTest, testUserRowEqual0) {
    vector<int32_t> value1{};
    vector<vector<int32_t>> value2{{1},{2},{3}};
    vector<MultiValueType<int32_t>> multiItemValues = genMultiValues(value2);
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(value1));
    unique_ptr<FeatureInput> input2(genMultiValueInput<int32_t>(multiItemValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_EQUAL);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(OverLapFeatureFunctionTest, testString) {
    vector<unique_ptr<FeatureInput>> userInput;
    vector<unique_ptr<FeatureInput>> itemInput;
    userInput = genStringFeatureInput(vector<vector<string>>{{"abc", "ab", "a"}});
    itemInput = genStringFeatureInput(vector<vector<string>>{{"abc", "a", "c"}, {"abc", "ab", "a"}});
    for(size_t i = 0; i < userInput.size(); ++i){
        for(size_t j = 0;j < itemInput.size(); ++j){
            vector<FeatureInput*> inputs;
            inputs.push_back(userInput[i].get());
            inputs.push_back(itemInput[j].get());
            genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
            checkMultiSparseFeatures(vector<string>{"fg_abc_a", "fg_abc_ab_a"}, vector<size_t>{0,1});
        }
    }
}

TEST_F(OverLapFeatureFunctionTest, testUserMultiRow) {
    vector<unique_ptr<FeatureInput>> userInput;
    vector<unique_ptr<FeatureInput>> itemInput;
    userInput = genStringFeatureInput(vector<vector<string>>{{"abc", "ab", "a"},{"ab", "c"}});
    itemInput = genStringFeatureInput(vector<vector<string>>{{"abc", "a", "c"}, {"abc", "ab", "a","c"}});
    for(size_t i = 0; i < userInput.size(); ++i){
        for(size_t j = 0;j < itemInput.size(); ++j){
            vector<FeatureInput*> inputs;
            inputs.push_back(userInput[i].get());
            inputs.push_back(itemInput[j].get());
            genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
            checkMultiSparseFeatures(vector<string>{"fg_abc_a", "fg_ab_c"}, vector<size_t>{0,1});
        }
    }
}

TEST_F(OverLapFeatureFunctionTest, testUserRowNotEqualItemRow) {
    vector<unique_ptr<FeatureInput>> userInput;
    vector<unique_ptr<FeatureInput>> itemInput;
    userInput = genStringFeatureInput(vector<vector<string>>{{"abc", "ab", "a"},{"ab", "a"}});
    itemInput = genStringFeatureInput(vector<vector<string>>{{"abc", "a", "c"}, {"abc", "ab", "a"},{"ab","a"}});
    for(size_t i = 0; i < userInput.size(); ++i){
        for(size_t j = 0;j < itemInput.size(); ++j){
            vector<FeatureInput*> inputs;
            inputs.push_back(userInput[i].get());
            inputs.push_back(itemInput[j].get());
            genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
            ASSERT_TRUE(_features.get() == nullptr);
        }
    }
}

TEST_F(OverLapFeatureFunctionTest, testDifferentInt) {
    vector<int32_t> value1{1,2,3};
    vector<vector<int64_t>> value2{{1,2,3},{3,2,1},{2}};
    vector<MultiValueType<int64_t>> multiItemValues = genMultiValues(value2);
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(value1, 1, 3));
    unique_ptr<FeatureInput> input2(genMultiValueInput<int64_t>(multiItemValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_EQUAL);
    checkMultiSparseFeatures(vector<string>{"fg_1", "fg_0", "fg_0"}, vector<size_t>{0,1,2});
}

TEST_F(OverLapFeatureFunctionTest, testCompareIntUserStringItem) {
    vector<int32_t> value1{123,2,123};
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(value1));

    vector<unique_ptr<FeatureInput>> itemInput;
    itemInput = genStringFeatureInput(vector<vector<string>>{{"1", "2", "3"}, {"abc", "ab", "a"},{"123"}});

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(itemInput[0].get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_EQUAL);
    checkMultiSparseFeatures(vector<string>{"fg_0", "fg_0", "fg_1"}, vector<size_t>{0,1,2});
}

TEST_F(OverLapFeatureFunctionTest, testCompareStringUserAndIntItem) {
    vector<unique_ptr<FeatureInput>> userInput;
    userInput = genStringFeatureInput(vector<vector<string>>{{"1", "2", "3"}, {"abc", "2", "a"},{"123"}});

    vector<int32_t> value2{123,2,123};
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(value2));

    vector<FeatureInput*> inputs;
    inputs.push_back(userInput[0].get());
    inputs.push_back(input2.get());
    genOverLapFeature(inputs, "fg", OverLapFeatureConfig::OT_COMMON_WORDS);
    checkMultiSparseFeatures(vector<string>{"fg_2", "fg_123"}, vector<size_t>{0,0,1});
}

TEST_F(OverLapFeatureFunctionTest, testDenseCommon) {
    testSimpleDense(vector<int32_t>{1,2,3},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_COMMON_WORDS,
                    {2 ,3});
}

TEST_F(OverLapFeatureFunctionTest, testDenseEqual) {
    testSimpleDense(vector<int32_t>{1,2,3},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_EQUAL,
                    {0 ,1});
}

TEST_F(OverLapFeatureFunctionTest, testDenseIsContain) {
    testSimpleDense(vector<int32_t>{1,2},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_CONTAIN,
                    {0 ,1});
}

TEST_F(OverLapFeatureFunctionTest, testDenseDiff) {
    testSimpleDense(vector<int32_t>{1,2,3},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_DIFF_WORDS,
                    {1 ,0});
}


TEST_F(OverLapFeatureFunctionTest, testDenseDiffBoth) {
    testSimpleDense(vector<int32_t>{1,2,3},
                    vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                    OverLapFeatureConfig::OT_DIFF_BOTH,
                    {2 ,0});
}

TEST_F(OverLapFeatureFunctionTest, testSparseCommon) {
    testSimpleSparse(vector<int32_t>{1,2,3},
                     vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                     OverLapFeatureConfig::OT_COMMON_WORDS,
                     {"fg_2" ,"fg_3"});
}

TEST_F(OverLapFeatureFunctionTest, testSparseDiffBoth0) {
    testSimpleSparse(vector<int32_t>{1,2,3},
                     vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                     OverLapFeatureConfig::OT_DIFF_BOTH,
                     {"fg_2" ,"fg_0"});
}

TEST_F(OverLapFeatureFunctionTest, testSparseDiffBothCut) {
    testSimpleSparse(vector<int32_t>{1,2,3},
                     vector<vector<int32_t>>{{1,3,5},{1,2,3}},
                     OverLapFeatureConfig::OT_DIFF_BOTH,
                     {"fg_1" ,"fg_0"},
                     1);
}

}
