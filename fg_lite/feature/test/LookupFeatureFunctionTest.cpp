
#include "fg_lite/feature/LookupFeatureFunction.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "autil/MultiValueCreator.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {

class LookupFeatureFunctionTest : public FeatureFunctionTestBase {
public:
    void genLookupFeatures(const vector<FeatureInput*> &inputs,
                           const string &prefix = "fg_",
                           bool needDiscrete = false,
                           const Normalizer &normalizer = Normalizer(),
                           const string &combiner = "mean",
                           bool needKey=true,
                           uint32_t dimension=1,
                           bool needWeighting = false,
                           bool isOptimized = false,
                           const std::string &defaultLookupResult = "",
                           bool hasDefault = false)
    {
        genFeatures(inputs, [=]() {
                    return new LookupFeatureFunction("name", prefix,
                            needDiscrete, needKey,
                            normalizer, combiner, dimension, needWeighting,
                            isOptimized, vector<float>(), defaultLookupResult, hasDefault);
                });
    }
    void genLookupFeaturesBoundary(const vector<FeatureInput*> &inputs,
                                   const vector<float> &boundaries,
                                   bool isOptimized)
    {
        genFeatures(inputs, [=]() {
                    return new LookupFeatureFunction("name", "fg_",
                            false, false,
                            Normalizer(), "mean", 1, false,
                            isOptimized, boundaries, "", false);
                });
    }
protected:
    template<typename ResultT = float, typename FeatureT = SingleDenseFeatures>
    void checkSingleDenseFeatures(const vector<ResultT> &results) {
        auto typedFeatures = ASSERT_CAST_AND_RETURN(FeatureT, _features.get());
        EXPECT_EQ(typedFeatures->_featureValues.size(), results.size());
        EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(results.data(), results.size()));
    }
    void checkMultiSparseFeatures(const vector<string> &names, vector<size_t> offsets) {
        vector<ConstString> constnames;
        for (auto name :names) {
            constnames.push_back(ConstString(name.c_str(), name.size()));
        }
        checkMultiSparseFeatures(constnames, offsets);
    }
    void checkMultiSparseFeatures(const vector<ConstString> &names, vector<size_t> offsets) {
        auto typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures, _features.get());
        EXPECT_EQ(names.size(), typedFeatures->_featureNames.size());
        EXPECT_THAT(typedFeatures->_featureNames, ElementsAreArray(names.data(), names.size()));
        EXPECT_THAT(typedFeatures->_offsets, ElementsAreArray(offsets.data(), offsets.size()));
    }
    void checkMultiSparseWeightingFeatures(
            vector<string> names,
            vector<double> values,
            vector<size_t> offsets)
    {
        MultiSparseWeightingFeatures * typedFeatures =
            ASSERT_CAST_AND_RETURN(MultiSparseWeightingFeatures, _features.get());
        EXPECT_EQ(names.size(), typedFeatures->_featureNames.size());
        EXPECT_EQ(names.size(), typedFeatures->_featureValues.size());
        EXPECT_EQ(values.size(), typedFeatures->_featureValues.size());
        for (size_t i = 0; i < names.size(); i++) {
            EXPECT_EQ(ConstString(names[i]), typedFeatures->_featureNames[i]);
            EXPECT_DOUBLE_EQ(values[i], typedFeatures->_featureValues[i]);
        }
        EXPECT_EQ(offsets.size(), typedFeatures->_offsets.size());
        for (size_t i = 0; i < offsets.size(); i++) {
            EXPECT_EQ(offsets[i], typedFeatures->_offsets[i]);
        }
    }
    void testSingleValue(const vector<string> &keys, const string &combiner, float expect) {
        auto multiValues = genMultiStringValues({{"k1:123", "k2:234", "k3:3"}});
        unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
        unique_ptr<FeatureInput> input2(genDenseInput<string>(keys, 1, keys.size()));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());

        genLookupFeatures(inputs, "fg_", false, Normalizer(), combiner);
        checkSingleDenseFeatures(vector<float>{expect});
    }

};

TEST_F(LookupFeatureFunctionTest, testMapFromUserAndKeyFromItem) {
    vector<string> mapValues{"k1:123", "k2:234", "k3:3"};
    vector<vector<string>> keyValues{{"k1"}, {"k2", "k3"}, {"k3"}};
    auto multiValues = genMultiStringValues(keyValues);
    unique_ptr<FeatureInput> mapInput(genDenseInput<string>(mapValues, 1, 3));
    unique_ptr<FeatureInput> keyInput(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(mapInput.get());
    inputs.push_back(keyInput.get());
    genLookupFeatures(inputs, "fg_", true);
    checkMultiSparseFeatures(vector<string>{"fg_k1_123", "fg_k2_234", "fg_k3_3", "fg_k3_3"},
                             vector<size_t>{0,1,3});
}

TEST_F(LookupFeatureFunctionTest, testMapFromUserAndKeyFromItemNonString) {
    vector<string> mapValues{"123:123", "234:234", "3:3"};
    vector<vector<int64_t>> keyValues{{123}, {234, 3}, {3}};
    auto multiInt64s = genMultiValues<int64_t>(keyValues);
    unique_ptr<FeatureInput> mapInput(genDenseInput<string>(mapValues, 1, 3));
    unique_ptr<FeatureInput> keyInput(genMultiValueInput<int64_t>(multiInt64s));
    vector<FeatureInput*> inputs;
    inputs.push_back(mapInput.get());
    inputs.push_back(keyInput.get());
    genLookupFeatures(inputs, "fg_", true);
    checkMultiSparseFeatures(vector<string>{"fg_123_123", "fg_234_234", "fg_3_3", "fg_3_3"},
                             vector<size_t>{0,1,3});
}


TEST_F(LookupFeatureFunctionTest, testSingleValue) {
    vector<string> keys1{"k1", "k2", "k3"};
    testSingleValue(keys1, "sum", 360);
    testSingleValue(keys1, "avg", 120);
    testSingleValue(keys1, "mean", 120);
    testSingleValue(keys1, "min", 3);
    testSingleValue(keys1, "max", 234);
    // no match results
    testSingleValue({}, "sum", 0);
    testSingleValue({}, "avg", 0);
    testSingleValue({}, "mean", 0);
    testSingleValue({}, "min", 0);
    testSingleValue({}, "max", 0);
}

TEST_F(LookupFeatureFunctionTest, testDiscrete) {
    vector<vector<string>> values1{{"k1:123"}, {"k2:234"}, {"k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs);
    checkSingleDenseFeatures(vector<float>{123,0,3});
    genLookupFeatures(inputs, "fg_", true);
    checkMultiSparseFeatures(vector<string>{"fg_k1_123", "fg_k3_3"},
                             vector<size_t>{0,1,1});
}

TEST_F(LookupFeatureFunctionTest, testDiscreteDefaultValue) {
    vector<vector<string>> values1{{"k1:123"}, {"k2:234"}, {"k3:3"}};
    auto multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k4", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 3, 1));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs, "fg_", true, Normalizer(), "", true, 1, false, false, "default", true);
    checkMultiSparseFeatures(vector<string>{"fg_k1_123", "fg_k4_default", "fg_k3_3"},
                             vector<size_t>{0,1,2});
}

TEST_F(LookupFeatureFunctionTest, testDiscreteWithWeighting) {
    vector<vector<string>> values1{{"k1:123"}, {"k2:234"}, {"k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs, "fg_", true, Normalizer(), "mean", true, 1, true, false);
    checkMultiSparseWeightingFeatures(
        vector<string>{"fg_k1", "fg_k3"},
        vector<double>{123, 3},
        vector<size_t>{0,1,1});
}

TEST_F(LookupFeatureFunctionTest, testDiscreteIsOptimized) {
    vector<vector<string>> values1{{"123"}, {"234"}, {"3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());

    genLookupFeatures(inputs, "fg_", true, Normalizer(), "mean", false, 1, false, true);
    checkMultiSparseFeatures(
            vector<string>{"fg_123", "fg_234", "fg_3"},
            vector<size_t>{0, 1, 2});
}

TEST_F(LookupFeatureFunctionTest, testNormalizeIsOptimized) {
    vector<vector<string>> values1{{"123", "3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=minmax,min=0.0,max=2.0"));

    genLookupFeatures(inputs, "fg_", false, normalizer, "mean", false, 1, false, true);
    checkSingleDenseFeatures(vector<float>{31.5});
}

TEST_F(LookupFeatureFunctionTest, testSimpleNeedDiscreteNoNeedKeyIsOptimized) {
    vector<vector<string>> values1{{"123"}, {"3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());


    genLookupFeatures(inputs, "fg_", false, Normalizer(), "mean", false, 1, false, true);
    checkSingleDenseFeatures(vector<float>{123,3});
    genLookupFeatures(inputs, "fg_", true, Normalizer(), "mean", false, 1, false, true);
    checkMultiSparseFeatures(vector<string>{"fg_123", "fg_3"},
                             vector<size_t>{0,1});
}

TEST_F(LookupFeatureFunctionTest, testOptimizedNeedkey) {
    vector<vector<string>> values1{{"123"}, {"234"}, {"3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());

    genLookupFeatures(inputs, "fg_", true, Normalizer(), "mean", true, 1, true, true);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(LookupFeatureFunctionTest, testNormalize) {
    vector<vector<string>> values1{{"k1:123", "k2:234", "k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=minmax,min=0.0,max=2.0"));

    genLookupFeatures(inputs, "fg_", false, normalizer);
    checkSingleDenseFeatures(vector<float>{31.5});
}

TEST_F(LookupFeatureFunctionTest, testSimpleNeedDiscreteNoNeedKey) {
    vector<vector<string>> values1{{"k1:123"}, {"k2:234"}, {"k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs);
    checkSingleDenseFeatures(vector<float>{123,0,3});
    genLookupFeatures(inputs, "fg_", true, Normalizer(), "mean", false);

    checkMultiSparseFeatures(vector<string>{"fg_123", "fg_3"},
                             vector<size_t>{0,1,1});
}

TEST_F(LookupFeatureFunctionTest, testUserNot1) {
    vector<vector<string>> values1{{"k1:123"}, {"k2:234"}, {"k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k2", "k3", "", "k4", "k1"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 3, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs);
    checkSingleDenseFeatures(vector<float>{123,0,0});
    genLookupFeatures(inputs, "fg_", true);
    checkMultiSparseFeatures(vector<string>{"fg_k1_123"},
                             vector<size_t>{0,1,1});
}

TEST_F(LookupFeatureFunctionTest, testUserInputInvalid) {
    vector<vector<string>> values1{{"k1:123"}, {"k2:234"}, {"k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k2", "k3", ""};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 2, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genLookupFeatures(inputs);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(LookupFeatureFunctionTest, testItemInvalid) {
    vector<vector<int64_t>> itemValues{{30068,29889},{},{20,30}};
    vector<autil::MultiValueType<int64_t>> multiItem = genMultiValues(itemValues);
    unique_ptr<FeatureInput> itemInput(genMultiValueInput<int64_t>(multiItem));

    vector<string> values2{"k1", "k2", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 3));
    vector<FeatureInput*> inputs;
    inputs.push_back(itemInput.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(LookupFeatureFunctionTest, testMulti) {
    vector<vector<string>> values1{{"k0:1", "k1:5", "k2:1",}, {"k2:2","k3:4", "k4:2"}, {"k3:1"}, {}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k2", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 3));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs, "", false, Normalizer(), "", true, 4, false);
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures,
            _features.get());
    vector<float> expects = {
        6, 0, 0, 0,
        6, 0, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 0,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 4, 8, 12));

    genLookupFeatures(inputs, "", false, Normalizer(), "", true, 2, false);
    typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures, _features.get());
    expects = {
        6, 0,
        6, 0,
        1, 0,
        0, 0,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 2, 4, 6));
}

TEST_F(LookupFeatureFunctionTest, testValueIsMultiValue) {
    vector<string> mapValues{"k1:1,2,3", "k2:4,5,6", "k3:7,8,9"};
    vector<vector<string>> keyValues{{"k1"}, {"k2"}, {"k3"}, {"k1"}};
    auto multiValues = genMultiStringValues(keyValues);
    unique_ptr<FeatureInput> mapInput(genDenseInput<string>(mapValues, 1, 3));
    unique_ptr<FeatureInput> keyInput(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(mapInput.get());
    inputs.push_back(keyInput.get());

    genLookupFeatures(inputs, "f1", false, Normalizer(), "max", true, 3, false);
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures, _features.get());
    vector<float> expects = {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9,
        1, 2, 3
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 3, 6, 9));

    genLookupFeatures(inputs, "f1", false, Normalizer(), "max", true, 2, false);
    typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures, _features.get());
    expects = {
        1, 2,
        4, 5,
        7, 8,
        1, 2,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 2, 4, 6));
}

TEST_F(LookupFeatureFunctionTest, testMultiKeyMultiValueSimple) {

    vector<string> mapValues {"k1:1,2,3", "k2:6,7,8", "k3:11,12,13"};
    unique_ptr<FeatureInput> input1(genDenseInput<string>(mapValues, 1, mapValues.size()));

    vector<vector<string>> keyValues{{"k1", "k2", "k3"}};
    auto multiValues = genMultiStringValues(keyValues);
    unique_ptr<FeatureInput> input2(genMultiValueInput<MultiChar>(multiValues));
    // unique_ptr<FeatureInput> input2(genDenseInput<string>(keyValues, 3, 1));

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs, "f1", false, Normalizer(), "sum", true, 4, false);
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures, _features.get());

    vector<float> expects = {
        18, 21, 24, 0,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0));
}

TEST_F(LookupFeatureFunctionTest, testMultiKeyMultiValue) {

    vector<vector<string>> mapValues {
        {"k1:1,2,3", "k2:6,7,8", "k3:11,12,13"},
        {"k1:1,2,3,4,5", "k2:6,7,8,9,10", "k3:11,12,13,14,15"},
        {"k1:1,2,3", "k2:4,5,6,7,8", "k3:9,10,11,12"},
        {"k1:1,2,3", "k2:4,5,6,7,8"},
    };
    vector<MultiString> values1 = genMultiStringValues(mapValues);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(values1));

    vector<vector<string>> keyValues{{"k1", "k2", "k3"}};
    auto values2 = genMultiStringValues(keyValues);
    unique_ptr<FeatureInput> input2(genMultiValueInput<MultiChar>(values2));

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeatures(inputs, "f1", false, Normalizer(), "sum", true, 4, false);
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures, _features.get());

    vector<float> expects = {
        18, 21, 24, 0,
        18, 21, 24, 27,
        14, 17, 20, 19,
        5, 7, 9, 7,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 4, 8, 12));
}



TEST_F(LookupFeatureFunctionTest, testMultiIsOptimized) {
    vector<vector<string>> values1{{"5", "1", "0"}, {"2","4", "2"}, {"1"}, {}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());

    genLookupFeatures(inputs, "", false, Normalizer(), "", false, 3, false, true);
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures,
            _features.get());
    vector<float> expects = {
        6, 0, 0,
        8, 0, 0,
        1, 0, 0,
        0, 0, 0,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 3, 6, 9));

    genLookupFeatures(inputs, "", false, Normalizer(), "", false, 2, false, true);
    typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures, _features.get());
    expects = {
        6, 0,
        8, 0,
        1, 0,
        0, 0,
    };
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(expects.data(), expects.size()));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 2, 4, 6));
}

TEST_F(LookupFeatureFunctionTest, testBucketize) {
    vector<vector<string>> values1{{"k1:123", "k2:234", "k3:3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<string> values2{"k1", "k3"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeaturesBoundary(inputs, vector<float>{0, 200, 400}, false);
    checkSingleDenseFeatures<int64_t, SingleIntegerFeatures>(vector<int64_t>{1});
}

TEST_F(LookupFeatureFunctionTest, testBucketizeOptimized) {
    vector<vector<string>> values1{{"123", "3"}};
    vector<MultiString> multiValues = genMultiStringValues(values1);
    unique_ptr<FeatureInput> input1(genMultiValueInput<MultiChar>(multiValues));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());

    genLookupFeaturesBoundary(inputs, vector<float>{0, 200, 400}, true);
    checkSingleDenseFeatures<int64_t, SingleIntegerFeatures>(vector<int64_t>{1});
}

}
