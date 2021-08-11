
#include "autil/MultiValueCreator.h"
#include "fg_lite/feature/ComboFeatureFunction.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace autil;
using namespace std;
using namespace testing;

namespace fg_lite {

class ComboFeatureFunctionTest : public FeatureFunctionTestBase {
public:
    FeatureFunction *createComboFeatureFunction(const string &prefix, size_t inputCount, const vector<bool> &pruneRight, const vector<int32_t> &pruneLimit, const bool needSort)
    {
        return new ComboFeatureFunction("name", prefix,
                pruneRight, pruneLimit, inputCount, needSort);
    }

    void genComboFeatures(vector<FeatureInput*> inputs,
                          const string &prefix = "pf_",
                          const vector<bool> pruneRight = {},
                          const vector<int32_t> pruneLimit = {},
                          const bool needSort = false)
    {
        genFeatures(inputs, [this, prefix, pruneRight, pruneLimit, needSort, &inputs]() {
                                return createComboFeatureFunction(prefix, inputs.size(), pruneRight, pruneLimit, needSort);
                });
    }
protected:
    void checkComboFeatures(const vector<FeatureInput*> &inputs,
                            const vector<string> &expectNames,
                            const vector<size_t> &expectOffsets,
                            const string &prefix = "pf_",
                            const vector<bool> pruneRight = {},
                            const vector<int32_t> pruneLimit = {},
                            const bool needSort = false)
    {
        genComboFeatures(inputs, prefix, pruneRight, pruneLimit, needSort);
        MultiSparseFeatures *typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures,
                _features.get());
        ASSERT_EQ(expectNames.size(), typedFeatures->_featureNames.size());
        for (size_t i = 0; i < expectNames.size(); i++) {
            EXPECT_EQ(ConstString(expectNames[i]), typedFeatures->_featureNames[i]);
        }
        ASSERT_EQ(typedFeatures->_offsets.size(), expectOffsets.size());
        for (size_t i = 0; i < expectOffsets.size(); i++) {
            EXPECT_EQ(expectOffsets[i], typedFeatures->_offsets[i]);
        }
    }
};

TEST_F(ComboFeatureFunctionTest, testMultiQinfo) {
    vector<int32_t> values1{0, 1, 2};
    vector<int32_t> values2{0};

    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1, 1, 3));
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(values2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    checkComboFeatures(inputs, {"pf_0_0", "pf_1_0", "pf_2_0"}, {0});
}

TEST_F(ComboFeatureFunctionTest, testNoPrefix) {
    vector<int32_t> values1{0, 1, 2};
    vector<int32_t> values2{0};

    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1, 1, 3));
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(values2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    checkComboFeatures(inputs, {"0_0", "1_0", "2_0"}, {0}, "");
}

TEST_F(ComboFeatureFunctionTest, testSimple) {
    vector<int32_t> values1{1,2,3};
    vector<string> values2{"abc", "bcd", "cde"};
    vector<vector<int32_t>> values3{{10,20},{30,40},{50,100}};

    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1));
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2));
    vector<autil::MultiValueType<int32_t>> multiValues = genMultiValues(values3);
    unique_ptr<FeatureInput> input3(genMultiValueInput<int32_t>(multiValues));

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    inputs.push_back(input3.get());

    checkComboFeatures(
            inputs,
            vector<string>{"pf_1_abc_10",
                    "pf_1_abc_20",
                    "pf_2_bcd_30",
                    "pf_2_bcd_40",
                    "pf_3_cde_50",
                    "pf_3_cde_100"
                    },
            vector<size_t>{0,2,4});
}

TEST_F(ComboFeatureFunctionTest, testAllSingle) {
    vector<int32_t> values1{1,2,3};
    vector<string> values2{"abc", "bcd", "cde"};
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1));
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    checkComboFeatures(
            inputs,
            vector<string>{"pf_1_abc", "pf_2_bcd", "pf_3_cde"},
            vector<size_t>{0,1,2});
}

TEST_F(ComboFeatureFunctionTest, testOneInput) {
    vector<int32_t> values1{1,2,3};
    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    checkComboFeatures(
            inputs,
            vector<string>{"pf_1", "pf_2", "pf_3"},
            vector<size_t>{0,1,2});
}

TEST_F(ComboFeatureFunctionTest, testInvalidValue) {
    vector<uint8_t> values1{1,255,3};
    vector<string> values2{"abc", "bcd", "cde"};
    vector<vector<uint8_t>> values3{{1},{1,2,3},{255,3}};

    unique_ptr<FeatureInput> input1(genDenseInput<uint8_t>(values1));
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2));
    vector<autil::MultiValueType<uint8_t>> multiValues = genMultiValues(values3);
    unique_ptr<FeatureInput> input3(genMultiValueInput<uint8_t>(multiValues));

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    inputs.push_back(input3.get());

    checkComboFeatures(
            inputs,
            vector<string>{"pf_1_abc_1", "pf_3_cde_3"},
            vector<size_t>{0,1,1});
}

TEST_F(ComboFeatureFunctionTest, testRowEqual1) {
    vector<string> values2{"abc", "bcd", "cde"};
    vector<vector<uint32_t>> values3{{1,2,3}};

    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2));
    vector<autil::MultiValueType<uint32_t>> multiValues = genMultiValues(values3);
    unique_ptr<FeatureInput> input3(genMultiValueInput<uint32_t>(multiValues));

    vector<FeatureInput*> inputs;
    inputs.push_back(input2.get());
    inputs.push_back(input3.get());

    checkComboFeatures(
            inputs,
            vector<string>{"pf_abc_1", "pf_abc_2", "pf_abc_3",
                    "pf_bcd_1", "pf_bcd_2", "pf_bcd_3",
                    "pf_cde_1", "pf_cde_2", "pf_cde_3"},
            vector<size_t>{0,3,6});
}

TEST_F(ComboFeatureFunctionTest, testDocInvalid) {
    vector<uint8_t> values1{1,3};
    vector<string> values2{"abc", "bcd", "cde"};

    unique_ptr<FeatureInput> input1(genDenseInput<uint8_t>(values1));
    unique_ptr<FeatureInput> input2(genDenseInput<string>(values2));

    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());
    genComboFeatures(inputs, "fg");
    ASSERT_TRUE(_features.get() == nullptr);
}

TEST_F(ComboFeatureFunctionTest, testPruneSimple) {
    vector<int32_t> values1{0, 1, 2};
    vector<int32_t> values2{0, 1, 2};

    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1, 1, 3));
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(values2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    checkComboFeatures(inputs, {"0_0", "1_0", "0_1", "1_1", "0_2", "1_2"}, {0,2,4},
                       "", {true, true}, {2, 4});
    checkComboFeatures(inputs, {"0_0", "1_0", "2_0", "0_1", "1_1", "2_1", "0_2", "1_2", "2_2"}, {0,3,6},
                       "", {true, true}, {10, 4});
    checkComboFeatures(inputs, {"1_0", "2_0", "1_1", "2_1", "1_2", "2_2"}, {0,2,4},
                       "", {false, true}, {2, 4});
    checkComboFeatures(inputs, {"0_0", "1_0", "2_0", "0_1", "1_1", "2_1", "0_2", "1_2", "2_2"}, {0,3,6},
                       "", {false, true}, {10, 4});

}

TEST_F(ComboFeatureFunctionTest, testPruneSorted) {
    vector<int32_t> values1{0, 1, 2};
    vector<int32_t> values2{0, 1, 2};

    unique_ptr<FeatureInput> input1(genDenseInput<int32_t>(values1, 1, 3));
    unique_ptr<FeatureInput> input2(genDenseInput<int32_t>(values2));
    vector<FeatureInput*> inputs;
    inputs.push_back(input1.get());
    inputs.push_back(input2.get());

    checkComboFeatures(inputs, {"0_0", "0_1", "0_1", "1_1", "0_2", "1_2"}, {0,2,4},
                       "", {true, true}, {2, 4}, true);
    checkComboFeatures(inputs, {"0_0", "0_1", "0_2", "0_1", "1_1", "1_2", "0_2", "1_2", "2_2"}, {0,3,6},
                       "", {true, true}, {10, 4}, true);
    checkComboFeatures(inputs, {"0_1", "0_2", "1_1", "1_2", "1_2", "2_2"}, {0,2,4},
                       "", {false, true}, {2, 4}, true);
    checkComboFeatures(inputs, {"0_0", "0_1", "0_2", "0_1", "1_1", "1_2", "0_2", "1_2", "2_2"}, {0,3,6},
                       "", {false, true}, {10, 4}, true);

}

}
