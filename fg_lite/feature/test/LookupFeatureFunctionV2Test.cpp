
#include "autil/MurmurHash.h"
#include "autil/MultiValueCreator.h"
#include "fg_lite/feature/LookupFeatureFunctionV2.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"
#include "fg_lite/feature/LookupFeatureSparseEncoder.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {

class LookupFeatureFunctionV2Test : public FeatureFunctionTestBase {
public:
    void genLookupFeaturesV2(vector<FeatureInput*> inputs,
                             const string &prefix = "fg",
                             const Normalizer &normalizer = Normalizer(),
                             const string &combiner = "sum",
                             uint32_t dimension = 1,
                             const vector<float> &boundaries = vector<float>(),
                             bool useHeader=false,
                             bool useSparse=false,
                             LookupFeatureV3KeyType keyType = LOOKUP_V3_KEY_HASH_0_TO_63_BIT,
                             LookupFeatureV3ValueType valueType = LOOKUP_V3_VALUE_ENCODE_32BIT
                             )
    {
        genFeatures(inputs, [=]() {
                                return new LookupFeatureFunctionV2(prefix, normalizer, combiner,
                                        dimension, boundaries, useHeader,
                                        useSparse, keyType, valueType);
                });
    }
protected:

    MultiChar createOneDoc(const map<ConstString, vector<float>> &kv,
                           uint32_t dimension=1,
                           bool useSparse=false,
                           LookupFeatureV3KeyType minHashType = LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
                           LookupFeatureV3ValueType appointedValueType = LOOKUP_V3_VALUE_ENCODE_32BIT)
    {
        string output;
        auto kvUnits = KvUnit::getKvUnitVec(kv, dimension);
        LookupFeatureEncoder::encodeMultiValue(kvUnits, dimension, output, minHashType,
                appointedValueType);

        vector<char> vc(output.begin(), output.end());
        char *mcBuffer = MultiValueCreator::createMultiValueBuffer(vc, _pool.get());
        MultiChar mc;
        mc.init(mcBuffer);
        return mc;
    }

    void testMatchForSparse(
            const vector<map<string, vector<float>>> &datas,
            const vector<string> &keys,
            const vector<float> &expects,
            const string &combiner = "sum",
            uint32_t dimension = 1,
            const vector<float> &boundaries = vector<float>(),
            LookupFeatureV3KeyType hashType = LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
            LookupFeatureV3ValueType valueType = LOOKUP_V3_VALUE_ENCODE_32BIT)
    {
        vector<MultiChar> values;
        vector<map<ConstString, vector<float>>> constStringDatas;
        for (const auto &data : datas) {
            map<ConstString, vector<float>> constStrMap;
            for (const auto &item : data) {
                constStrMap[ConstString(item.first)] = item.second;
            }
            constStringDatas.push_back(constStrMap);
        }

        for (const auto &data : constStringDatas) {
            std::string output;
            LookupFeatureSparseEncoder::encode(data, dimension, output, hashType, valueType);
            char *mcBuffer = MultiValueCreator::createMultiValueBuffer(output.data(),
                    output.size(), _pool.get());
            MultiChar mc;
            mc.init(mcBuffer);
            values.push_back(mc);
        }

        unique_ptr<FeatureInput> input1(genDenseInput<MultiChar>(values));
        unique_ptr<FeatureInput> input2(genDenseInput<string>(keys, 1, keys.size()));

        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());

        genLookupFeaturesV2(inputs, "", Normalizer(), combiner, dimension, boundaries,
                            true, true, hashType, valueType);
        auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleDenseFeatures, _features.get());

        ASSERT_EQ(typedFeatures->_featureValues.size(), expects.size());
        for (size_t i = 0; i < expects.size(); i++) {
            EXPECT_FLOAT_EQ(typedFeatures->_featureValues[i], expects[i]);
        }
        if (dimension > 1) {
            vector<size_t> expectoffsets;
            expectoffsets.resize(constStringDatas.size());
            std::generate(expectoffsets.begin(), expectoffsets.end(),
                          [n = 0, dimension] () mutable { n += dimension; return n - dimension; });
            auto multiFeature = dynamic_cast<MultiDenseFeatures*>(typedFeatures);
            EXPECT_THAT(multiFeature->_offsets,
                        ElementsAreArray(expectoffsets.data(), expectoffsets.size()));
        }
    }

    void testMatch(
            const vector<map<string, vector<float>>> &datas,
            const vector<string> &keys,
            const vector<float> &expects,
            const string &combiner = "sum",
            uint32_t dimension = 1,
            const vector<float> &boundaries = vector<float>(),
            bool useHeader=false,
            bool useSparse=false,
            LookupFeatureV3KeyType minHashType = LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
            LookupFeatureV3ValueType appointedValueType = LOOKUP_V3_VALUE_ENCODE_32BIT)
    {
        vector<MultiChar> values;
        vector<map<ConstString, vector<float>>> constStringDatas;
        for (const auto &data : datas) {
            map<ConstString, vector<float>> constStrMap;
            for (const auto &item : data) {
                constStrMap[ConstString(item.first)] = item.second;
            }
            constStringDatas.push_back(constStrMap);
        }

        for (const auto &data : constStringDatas) {
            values.push_back(createOneDoc(data, dimension));
        }

        unique_ptr<FeatureInput> input1(genDenseInput<MultiChar>(values));
        unique_ptr<FeatureInput> input2(genDenseInput<string>(keys, 1, keys.size()));

        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());

        genLookupFeaturesV2(inputs, "", Normalizer(), combiner, dimension, boundaries,
                            useHeader, useSparse, minHashType, appointedValueType);
        auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleDenseFeatures, _features.get());
        ASSERT_EQ(typedFeatures->_featureValues.size(), expects.size());
        for (size_t i = 0; i < expects.size(); i++) {
            EXPECT_FLOAT_EQ(typedFeatures->_featureValues[i], expects[i]);
        }
        if (dimension > 1) {
            vector<size_t> expectoffsets;
            expectoffsets.resize(constStringDatas.size());
            std::generate(expectoffsets.begin(), expectoffsets.end(),
                          [n = 0, dimension] () mutable { n += dimension; return n - dimension; });
            auto multiFeature = dynamic_cast<MultiDenseFeatures*>(typedFeatures);
            EXPECT_THAT(multiFeature->_offsets,
                        ElementsAreArray(expectoffsets.data(), expectoffsets.size()));
        }
    }

    void testIntegerMatch(
            const vector<map<string, vector<float>>> &datas,
            const vector<string> &keys,
            const vector<float> &expects,
            const string &combiner = "sum",
            uint32_t dimension = 1,
            const vector<float> &boundaries = vector<float>(),
            bool useHeader=false,
            bool useSparse=false,
            LookupFeatureV3KeyType minHashType = LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
            LookupFeatureV3ValueType appointedValueType = LOOKUP_V3_VALUE_ENCODE_32BIT)
    {
        vector<MultiChar> values;
        vector<map<ConstString, vector<float>>> constStringDatas;
        for (const auto &data : datas) {
            map<ConstString, vector<float>> constStrMap;
            for (const auto &item : data) {
                constStrMap[ConstString(item.first)] = item.second;
            }
            constStringDatas.push_back(constStrMap);
        }
        for (const auto &data : constStringDatas) {
            values.push_back(createOneDoc(data, dimension));
        }
        unique_ptr<FeatureInput> input1(genDenseInput<MultiChar>(values));
        unique_ptr<FeatureInput> input2(genDenseInput<string>(keys, 1, keys.size()));

        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genLookupFeaturesV2(inputs, "", Normalizer(), combiner, dimension, boundaries, useHeader,
                            useSparse, minHashType, appointedValueType);
        auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleIntegerFeatures, _features.get());
        ASSERT_EQ(typedFeatures->_featureValues.size(), expects.size());
        for (size_t i = 0; i < expects.size(); i++) {
            EXPECT_FLOAT_EQ(typedFeatures->_featureValues[i], expects[i]);
        }
        if (dimension > 1) {
            vector<size_t> expectoffsets;
            expectoffsets.resize(constStringDatas.size());
            std::generate(expectoffsets.begin(), expectoffsets.end(),
                          [n = 0, dimension] () mutable { n += dimension; return n - dimension; });
            auto multiFeature = dynamic_cast<MultiDenseFeatures*>(typedFeatures);
            EXPECT_THAT(multiFeature->_offsets,
                        ElementsAreArray(expectoffsets.data(), expectoffsets.size()));
        }
    }
};

TEST_F(LookupFeatureFunctionV2Test, testSparse) {
    vector<map<string, vector<float>>> testInput{
        {{"1", {0.1, 0.2}}, {"a", {0.3, 0.4}}, {"b", {0.5, 0.6}}},
        {{"b", {0, 0.2}}, {"4", {0.4}}, {"a", {0.6}}},
        {{"3", {0.3}}, {"6", {0.6}}, {"9", {0.9}}},
        {}
    };
    testMatchForSparse(testInput,
                       {"b", "a"},
                       {0.8, 1.0, 0.6, 0.2, 0, 0, 0, 0}, "sum", 2, {},
                       LOOKUP_V3_KEY_HASH_0_TO_31_BIT,
                       LOOKUP_V3_VALUE_ENCODE_32BIT);
}

TEST_F(LookupFeatureFunctionV2Test, testSingle) {
    // data value 中有float， 按理说使用 int 编码不应该过
    vector<map<string, vector<float>>> datas{
                   {{"1", {0.1}}, {"a", {0.2}}, {"b", {0.3}}},
                   {{"2", {0.2}}, {"4", {0.4}}, {"a", {0.6}}},
                   {{"3", {0.3}}, {"6", {0.6}}, {"9", {0.9}}},
                   {},
                   };
    vector<string> keys = {"b", "a"};
    testMatch(datas, keys, {0.5, 0.6, 0.0, 0.0}, "sum", 1, {}, true, false);
    testMatch(datas, keys, {0.25, 0.6, 0.0, 0.0}, "mean", 1, {}, true, false);
    testMatch(datas, keys, {0.25, 0.6, 0.0, 0.0}, "avg", 1, {}, true, false);
    testMatch(datas, keys, {0.5, 0.6, 0.0, 0.0}, "sum", 1, {}, true, false);
    testMatch(datas, keys, {0.2, 0.6, 0.0, 0.0}, "min", 1, {}, true, false);
    testMatch(datas, keys, {0.3, 0.6, 0.0, 0.0}, "max", 1, {}, true, false);
# define TEST_KEY_HASH_TYPE(KEY_HASH_TYPE)                              \
    testMatch(datas, keys, {0.3, 0.6, 0.0, 0.0}, "max", 1, vector<float>(), \
              true, false, KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT);  \
     testMatch(datas, keys, {0.3, 0.6, 0.0, 0.0}, "max", 1, vector<float>(), \
               true, false, KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT);             \
     testMatch(datas, keys, {0.3, 0.6, 0.0, 0.0}, "max", 1, vector<float>(), \
               true, false, KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT);             \
     testMatch(datas, keys, {0.3, 0.6, 0.0, 0.0}, "max", 1, vector<float>(), \
               true, false, KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_AUTO);

    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_0_TO_15_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT);

#undef TEST_KEY_HASH_TYPE
}

TEST_F(LookupFeatureFunctionV2Test, testMulti) {
    vector<map<string, vector<float>>> testInput{
                   {{"1", vector<float>({0.1, 0.0})}, {"a", vector<float>({0.2, 0.0})}, {"b", vector<float>({0.3, 0.0})}},
                   {{"2", vector<float>({0.2, 0.0})}, {"4", vector<float>({0.4, 0.0})}, {"a", vector<float>({0.6, 0.0})}},
                   {{"3", vector<float>({0.3, 0.0})}, {"6", vector<float>({0.6, 0.0})}, {"9", vector<float>({0.9, 0.0})}},
                   {},
    };
    testMatch(testInput, {"b", "a"}, {0.5, 0.0, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0},
              "", 2, {}, true, false);

# define TEST_KEY_HASH_TYPE(KEY_HASH_TYPE)  \
     testMatch(testInput, {"b", "a"}, {0.5, 0.0, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0},  \
               "", 2, vector<float>(), true, false,  \
               KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_8BIT);  \
     testMatch(testInput, {"b", "a"}, {0.5, 0.0, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0},  \
               "", 2, vector<float>(), true, false,  \
               KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_16BIT);  \
     testMatch(testInput, {"b", "a"}, {0.5, 0.0, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0},  \
               "", 2, vector<float>(), true, false,  \
               KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_32BIT);  \
     testMatch(testInput, {"b", "a"}, {0.5, 0.0, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0},  \
               "", 2, vector<float>(), true, false,  \
               KEY_HASH_TYPE, LOOKUP_V3_VALUE_ENCODE_AUTO);


    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_0_TO_15_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_16_TO_31_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_32_TO_47_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_48_TO_63_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
    TEST_KEY_HASH_TYPE(LOOKUP_V3_KEY_HASH_0_TO_63_BIT);

#undef TEST_KEY_HASH_TYPE
}

TEST_F(LookupFeatureFunctionV2Test, testUserInvalid) {
    vector<string> values1{"a", "b"};
    unique_ptr<FeatureInput> input1(genDenseInput<string>(values1, 1, 2));

    vector<string> values2{"a", "b"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 2, 1));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeaturesV2(inputs);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(LookupFeatureFunctionV2Test, testItemInvalid) {
    vector<string> values1{"a", "b"};
    unique_ptr<FeatureInput> input1(genDenseInput<string>(values1, 1, 2));

    vector<string> values2{"a", "b"};
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2, 1, 2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    genLookupFeaturesV2(inputs);
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(LookupFeatureFunctionV2Test, testBucketize) {

    vector<map<string, vector<float>>> data {
        {{"1", {0.1}}, {"a", {0.2}}, {"b", {0.3}}},
        {{"2", {0.2}}, {"4", {0.4}}, {"a", {0.6}}},
        {{"3", {0.3}}, {"6", {0.6}}, {"9", {0.9}}},
        {}
    };
    testIntegerMatch(data , {"b", "a"}, {4, 5, 1, 1}, "sum", 1,
                      {0, 0.2, 0.3, 0.5, 0.6, 0.7}, true, false);
}

}
