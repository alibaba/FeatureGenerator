#include "gtest/gtest.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "fg_lite/feature/RawFeatureFunction.h"

using namespace std;
using namespace autil;
using namespace testing;

namespace fg_lite {

class RawFeatureFunctionTest : public FeatureFunctionTestBase {
protected:
    FeatureFunction *createFeatureFunction(const string &name,
            const Normalizer &normalizer,
            const vector<float> &boundaries)
    {
        return new RawFeatureFunction(name, normalizer, boundaries, 1);
    }

    void genRawFeature(FeatureInput *input,
                       const string &name = "raw_feature",
                       const Normalizer &normalizer = Normalizer(),
                       const vector<float> &boundaries = vector<float>())
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input);
        genFeatures(inputs, [this, name, normalizer, boundaries]() {
                    return createFeatureFunction(name, normalizer, boundaries);
                });
    }

    template <typename T, typename ResultT, typename FeatureT>
    void testMultiValueInput(const vector<vector<T>> &values,
                             const vector<ResultT> &results,
                             const vector<int32_t> &offsets,
                             const vector<float> &boundaries = vector<float>())
    {
        vector<autil::MultiValueType<T>> multiValues = genMultiValues(values);
        unique_ptr<FeatureInput> input(genMultiValueInput<T>(multiValues));
        genRawFeature(input.get(), "feature", Normalizer(), boundaries);
        auto typedFeatures = ASSERT_CAST_AND_RETURN(FeatureT, _features.get());
        EXPECT_EQ(typedFeatures->_featureValues.size(), results.size());
        EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(results.data(), results.size()));
        EXPECT_THAT(typedFeatures->_offsets, ElementsAreArray(offsets.data(), offsets.size()));
    }
    template<typename T, typename ResultT, typename FeatureT>
    void testOutput(const vector<T> &input,
                    const vector<ResultT> &output,
                    const vector<float> &boundaries = vector<float>())
    {
        unique_ptr<FeatureInput> featureInput(genDenseInput<T>(input));
        genRawFeature(featureInput.get(), "feature", Normalizer(), boundaries);
        ASSERT_TRUE(_features.get() != nullptr);
        auto typedFeatures = ASSERT_CAST_AND_RETURN(FeatureT, _features.get());
        EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(output.data(), output.size()));
    }
};

TEST_F(RawFeatureFunctionTest, testSimple) {
    testOutput<int32_t, float, SingleDenseFeatures>(vector<int32_t>{40,50,60}, vector<float>{40, 50, 60});
}

TEST_F(RawFeatureFunctionTest, testLogNormalizer) {
    vector<int32_t> values{10, 100, 200};
    unique_ptr<FeatureInput> input(genDenseInput<int32_t>(values));
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=log10"));
    string name = "raw_feature";
    genRawFeature(input.get(), name, normalizer);
    ASSERT_TRUE(_features.get() != nullptr);
    SingleDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(SingleDenseFeatures, _features.get());
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAre(1.0f, 2.0f, 2.30103f));
}

TEST_F(RawFeatureFunctionTest, testMulti) {
    testMultiValueInput<int32_t, float, MultiDenseFeatures>(
            vector<vector<int32_t>>{{10},{20, 30, 40}, {50, 60}},
            vector<float>{10, 20, 30, 40, 50, 60},
            vector<int32_t>{0, 1, 4});
}

TEST_F(RawFeatureFunctionTest, testMultiString) {
    auto multiValues = genMultiStringValues({{"123"}, {"132","245"},{"abc"}});
    unique_ptr<FeatureInput> input(genMultiValueInput<MultiChar>(multiValues));
    genRawFeature(input.get());
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures,
            _features.get());
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAre(123.0f, 132.0f, 245.0f, 0.0f));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0, 1, 3));
}

TEST_F(RawFeatureFunctionTest, testMultiString2) {
    unique_ptr<FeatureInput> input(genDenseInput(vector<string>{"1", "2", "3"}, 1, 3));
    genRawFeature(input.get());
    MultiDenseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiDenseFeatures,
            _features.get());
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAre(1, 2, 3));
    EXPECT_THAT(typedFeatures->_offsets, ElementsAre(0));
}

TEST_F(RawFeatureFunctionTest, testIntegerOutput) {
    testOutput<int8_t, int64_t, SingleIntegerFeatures>(vector<int8_t>{1,2,3,4,5,6,7,8,9}, vector<int64_t>{0,1,1,1,2,2,2,3,3}, vector<float>{2.0f, 5.0f, 8.0f});
    testOutput<float, int64_t, SingleIntegerFeatures>(vector<float>{1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f}, vector<int64_t>{0,1,1,1,2,2,2,3,3}, vector<float>{2.0f, 5.0f, 8.0f});
    testOutput<string, int64_t, SingleIntegerFeatures>(vector<string>{"1","2","3","4","5","6","7","8","9"}, vector<int64_t>{0,1,1,1,2,2,2,3,3}, vector<float>{2.0f, 5.0f, 8.0f});
    testMultiValueInput<int32_t, int64_t, MultiIntegerFeatures>(
            vector<vector<int32_t>>{{1,2,3}, {4,5,6},{}, {7,8,9}},
            vector<int64_t>{0,1,1,1,2,2,0,2,3,3},
            vector<int32_t>{0, 3, 6, 7},
            vector<float>{2.0f, 5.0f, 8.0f});
}

TEST_F(RawFeatureFunctionTest, testZeroBucketize) {
    unique_ptr<FeatureInput> input(genDenseInput(vector<string>{}, 1, 0));
    genRawFeature(input.get(), "feature", Normalizer(), {-1, 0, 1});
    ASSERT_TRUE(_features.get() != nullptr);
    auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleIntegerFeatures, _features.get());
    vector<int> output = {2};
    EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(output.data(), output.size()));
}

}
