#include "fg_lite/feature/LookupFeatureFunctionArray.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {
const std::string &kCombiner = "none";
float kTimediff = -1.0f;
bool kNeedCombo = false;
int kCut1 = -1;
int kCut2 = -1;

class LookupFeatureFunctionArrayTest : public FeatureFunctionTestBase {
 public:
    void genLookupFeatures(const vector<FeatureInput*> &inputs,
                           const string &prefix,
                           const string &defaultLookupResult,
                           bool hasDefault,
                           bool needDiscrete,
                           const std::string& combiner = kCombiner,
                           float timediff = kTimediff,
                           bool needCombo = kNeedCombo,
                           int cut1 = kCut1,
                           int cut2 = kCut2,
                           const std::vector<float> &boundaries= {})
    {
        genFeatures(inputs, [=]() {
                    return new LookupFeatureFunctionArray("name", prefix,
                            defaultLookupResult, hasDefault, needDiscrete, boundaries, combiner, timediff, combiner,
                            needCombo, cut1, cut2);
                });
    }

    void runTest(FeatureInput *input1, FeatureInput *input2, FeatureInput *input3,
                 const vector<string> &values, const vector<size_t> &offset,
                 bool hasDefault = true)
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        genLookupFeatures(inputs, "fg_", "", hasDefault, true, "none", -1, false, -1, -1);
        delete input1;
        delete input2;
        delete input3;
        checkMultiSparseFeatures(values, offset);
    }

    void runTest(FeatureInput *input1, FeatureInput *input2, FeatureInput *input3,
                 FeatureInput *input4, FeatureInput *input5,
                 const vector<string> &values, const vector<size_t> &offset,
                 bool hasDefault = true, const std::string& combiner = "none", float timediff = -1.0f)
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        inputs.push_back(input4);
        inputs.push_back(input5);
        genLookupFeatures(inputs, "fg_", "", hasDefault, true, combiner, timediff);
        delete input1;
        delete input2;
        delete input3;
        delete input4;
        delete input5;
        checkMultiSparseFeatures(values, offset);
    }
    
    void runDenseTest(FeatureInput *input1, FeatureInput *input2, FeatureInput *input3,
                 FeatureInput *input4, FeatureInput *input5,
                 const vector<float> &values,
                 bool hasDefault = true, const std::string& combiner = "none", float timediff = -1.0f, int cut1 = -1)
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        inputs.push_back(input4);
        inputs.push_back(input5);
        genLookupFeatures(inputs, "fg_", "", hasDefault, false, combiner, timediff, false, cut1);
        delete input1;
        delete input2;
        delete input3;
        delete input4;
        delete input5;
        checkSingleDenseFeatures(values);
    }

    void runSingleSparseTest(FeatureInput *input1, FeatureInput *input2, FeatureInput *input3,
                 FeatureInput *input4, FeatureInput *input5,
                 const vector<std::string> &values,
                 bool hasDefault = true, const std::string& combiner = "none", float timediff = -1.0f)
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        inputs.push_back(input4);
        inputs.push_back(input5);
        genLookupFeatures(inputs, "fg_", "", hasDefault, true, combiner, timediff);
        delete input1;
        delete input2;
        delete input3;
        delete input4;
        delete input5;
        checkSingleSparseFeatures(values);
    }

    void runSingleSparseTest(FeatureInput *input1, FeatureInput *input2, FeatureInput *input3,
                 FeatureInput *input4, FeatureInput *input5,
                 FeatureInput *input6, FeatureInput *input7, FeatureInput* input8,
                 const vector<std::string> &values,
                 bool hasDefault = true, const std::string& combiner = "none", float timediff = -1.0f,
                 bool needCombo = false )
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        inputs.push_back(input4);
        inputs.push_back(input5);
        inputs.push_back(input6);
        inputs.push_back(input7);
        inputs.push_back(input8);
        genLookupFeatures(inputs, "fg_", "", hasDefault, true, combiner, timediff, needCombo);
        delete input1;
        delete input2;
        delete input3;
        delete input4;
        delete input5;
        delete input6;
        delete input7;
        delete input8;
        checkSingleSparseFeatures(values);
    }

    void runTestDense(FeatureInput *input1, FeatureInput *input2, FeatureInput *input3,
                      const vector<float> &values,
                      const std::vector<float> &boundaries = {})
    {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        genLookupFeatures(inputs, "fg_", "", false, false, kCombiner, kTimediff, kNeedCombo, kCut1,
                          kCut2, boundaries);
        delete input1;
        delete input2;
        delete input3;
        if (boundaries.empty()) {checkSingleDenseFeatures(values);} else {checkSingleIntegerFeatures(values);}
    }
};

TEST_F(LookupFeatureFunctionArrayTest, testIntKeys) {
    runTest(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
            genDenseInput<string>({"1", "2", "3"}, 3, 1),
            genDenseInput<int32_t>({1, 3, 5}, 3, 1),
            {"fg_", "fg_3", "fg_1"}, {0, 1, 2});
}

TEST_F(LookupFeatureFunctionArrayTest, testIntValues) {
    runTest(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
            genDenseInput<int32_t>({1, 2, 3}, 3, 1),
            genDenseInput<int32_t>({1, 3, 5}, 3, 1),
            {"fg_", "fg_3", "fg_1"}, {0, 1, 2});
}

TEST_F(LookupFeatureFunctionArrayTest, testIntKeyStringMapKey) {
    runTest(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
            genDenseInput<string>({"1", "2", "3"}, 3, 1),
            genDenseInput<int32_t>({1, 3, 5}, 3, 1),
            {"fg_", "fg_3", "fg_1"}, {0, 1, 2});
}

TEST_F(LookupFeatureFunctionArrayTest, testStringKeyIntMapKey1) {
    runTest(genDenseInput<std::string>({"4", "50006079", "3"}, 1, 3),
            genDenseInput<int32_t>({1, 2, 3}, 1, 3),
            genMultiValueInput(genMultiStringValues({{"50006079"}})),
            {"fg_2"}, {0}, false);
}

TEST_F(LookupFeatureFunctionArrayTest, testStringKeyIntMapKey) {
    runTest(genDenseInput<std::string>({"5", "4", "3"}, 3, 1),
            genDenseInput<int32_t>({1, 2, 3}, 3, 1),
            genMultiValueInput(genMultiStringValues({{}, {"5", "0", "4"}, {"5"}})),
            {"fg_1", "fg_2", "fg_1"}, {0, 0, 2}, false);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiValueKey) {
    runTest(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
            genDenseInput<int32_t>({1, 2, 3}, 3, 1),
            genMultiValueInput(genMultiValues<int64_t>({{}, {5, 0, 4}, {5}})),
            {"fg_1", "fg_2", "fg_1"}, {0, 0, 2}, false);
}

TEST_F(LookupFeatureFunctionArrayTest, testDenseMultiLookup) {
    runTestDense(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
                 genDenseInput<int32_t>({1, 2, 3}, 3, 1),
                 genMultiValueInput(genMultiValues<int32_t>({{}, {5, 0, 4}, {5}})),
                 {0, 3, 1});
}

TEST_F(LookupFeatureFunctionArrayTest, testDenseTypeConvert) {
    runTestDense(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
                 genDenseInput<string>({"1", "2", "3"}, 3, 1),
                 genDenseInput<int32_t>({1, 3, 5}, 3, 1),
                 {0, 3, 1});
}
TEST_F(LookupFeatureFunctionArrayTest, testDenseWithBucket) {
    runTestDense(genDenseInput<int32_t>({5, 4, 3}, 3, 1),
                 genDenseInput<int32_t>({6, 2, 3}, 3, 1),
                 genDenseInput<int32_t>({1, 3, 5}, 3, 1), 
                 {1, 2, 3}, {0, 2, 5, 9});
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffCount) {
    runDenseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int64_t>({4, 3, 5}, 3, 1),
                 genDenseInput<float>({3.0f}, 1, 1),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 {2, 1, 0},
                 true,
                 "count", 2.0f);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffMax) {
    runDenseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int32_t>({4, 3, 5}, 3, 1),
                 genDenseInput<float>({10000.0f}, 1, 1),
                 genDenseInput<float>({0.0f, 1000.0f, 7000.0f, 9000.0f}, 1, 4),
                 {6.0, 4.0, 0.0},
                 true,
                 "gap_max", 86400.0f);
}


TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffNoneString) {
    runTest(genDenseInput<std::string>({"k1"}, 1, 1),
                 genDenseInput<std::string>({"1"}, 1, 1),
                 genDenseInput<std::string>({"k1"}, 1, 1),
                 genDenseInput<std::string>({"1571043000"}, 1, 1),
                 genDenseInput<std::string>({"1571043000"}, 1, 1),
                 {"fg_1"}, {0},
                 true,
                 "none", 0);
}

TEST_F(LookupFeatureFunctionArrayTest, testSingleSparseNeedCombo) {
    runSingleSparseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int32_t>({4, 3, 5}, 3, 1),
                 genDenseInput<float>({3.0f}, 1, 1),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int32_t>({0, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 {std::string("fg_2_2"), std::string("fg_1_1"), std::string("fg_1_0")},
                 true,
                 std::string("count"), 10.0f, true);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffMinReal) {
    runDenseTest(genDenseInput<int32_t>({112912381,150920153,128733343,58499649,128733343,103545912,162971057,319776248,101145860,35060648,101145860,103890421,128733343,103890421,128733343,319776248,58499649,162971057,103890421,196639903,150920153,103545912,128733343,183203059,101737340,103890421,103890421,57785091,101737340,150920153,128733343,128733343,101737340,162971057,58499649,150920153,67097224,128733343,150920153,67097224,111212422,67097224,106117073}, 1, 43),
                 genDenseInput<double>({1571357716.0, 1571357716.0, 1571357716.0, 1571357828.0, 1571357832.0, 1571357836.0, 1571357840.0, 1571357850.0, 1571357850.0, 1571357850.0, 1571357854.0, 1571357854.0, 1571357854.0, 1571357919.0, 1571358016.0, 1571358021.0, 1571358032.0, 1571358038.0, 1571358092.0, 1571358116.0, 1571358122.0, 1571358128.0, 1571358132.0, 1571358136.0, 1571359626.0, 1571359626.0, 1571359626.0, 1571359644.0, 1571359644.0, 1571359644.0, 1571359702.0, 1571359731.0, 1571359804.0, 1571359816.0, 1571359827.0, 1571357749.0, 1571357893.0, 1571358040.0, 1571358190.0, 1571359705.0, 1571359735.0, 1571359765.0, 1571359819.0}, 1, 43),
                 genDenseInput<int32_t>({319776248}, 1, 1),
                 genDenseInput<double>({1571359834.0}, 1, 1),
                 genDenseInput<double>({1571357716.0, 1571357716.0, 1571357716.0, 1571357828.0, 1571357832.0, 1571357836.0, 1571357840.0, 1571357850.0, 1571357850.0, 1571357850.0, 1571357854.0, 1571357854.0, 1571357854.0, 1571357919.0, 1571358016.0, 1571358021.0, 1571358032.0, 1571358038.0, 1571358092.0, 1571358116.0, 1571358122.0, 1571358128.0, 1571358132.0, 1571358136.0, 1571359626.0, 1571359626.0, 1571359626.0, 1571359644.0, 1571359644.0, 1571359644.0, 1571359702.0, 1571359731.0, 1571359804.0, 1571359816.0, 1571359827.0, 1571357749.0, 1571357893.0, 1571358040.0, 1571358190.0, 1571359705.0, 1571359735.0, 1571359765.0, 1571359819.0}, 1, 43),
                 {5.0},
                 true,
                 "gap_min", 86400.0f);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffMinReal2) {
    runDenseTest(genDenseInput<int32_t>({129587992,145252317,334248005,129587992,354386549,145252317,156995890,283667131,390854500,108535201,129587992,390854500,390854500,65287858,145252317,259230497,562104370,152406706,145929734,152406706,390854500,580118568,129473139,244306003,129587992,477065317,129587992,129587992,129587992,129587992,62791271,62791271,129587992,129587992,129587992,129587992,129587992,162886045,187442392,593801875,408821011,187442392,116874112,244535080,187442392,259230497,122388571,108673394,390854500,108180166,257862768,57895461,239751641,57895461,129587992,244535080,213275378,562104370,57895461,129587992,390854500,57895461,213275378,129587992,477065317,65287858,129473139,213275378,221640519,221640519,221640519,446361363,221640519,129587992,145252317,213973898,187442392,116584650,65287858,101954829,390854500,129587992,145252317,129587992,152406706,129964656,385246447,168032343,129587992,129587992,129587992,129587992,187442392,187442392,586296713,115896926,187442392,187442392,244535080,187442392,330153209,385246447,108673394,57895461,257862768,1450471,239751641,239751641,57895461,129587992,244535080,586296713,1450471,213275378,552984591,140857930,221640519,36317362,145252317,187442392}, 1, 120),
                 genDenseInput<std::string>({"1571319156", "1571319156", "1571319156", "1571319161", "1571319161", "1571319161", "1571319204", "1571319222", "1571319222", "1571319222", "1571394459", "1571394459", "1571394459", "1571394482", "1571394494", "1571394500", "1571394507", "1571394507", "1571394507", "1571394549", "1571394563", "1571394571", "1571394578", "1571394616", "1571394620", "1571394620", "1571394620", "1571394637", "1571394637", "1571394637", "1571394637", "1571394637", "1571394637", "1571394899", "1571394998", "1571395113", "1571395113", "1571395113", "1571395132", "1571395132", "1571395132", "1571395193", "1571395205", "1571395223", "1571395223", "1571395223", "1571395279", "1571395279", "1571395279", "1571395305", "1571395313", "1571395326", "1571395339", "1571395357", "1571395396", "1571395406", "1571396950", "1571396964", "1571396988", "1571397003", "1571397003", "1571397003", "1571397043", "1571397043", "1571397043", "1571397062", "1571397076", "1571397076", "1571397076", "1571397088", "1571397213", "1571397213", "1571397213", "1571397221", "1571397221", "1571397221", "1571397232", "1571319181", "1571319211", "1571319238", "1571319238", "1571394503", "1571394503", "1571394533", "1571394533", "1571394594", "1571394624", "1571394624", "1571394686", "1571394932", "1571394932", "1571395023", "1571395084", "1571395144", "1571395174", "1571395174", "1571395205", "1571395235", "1571395265", "1571395265", "1571395265", "1571395325", "1571395325", "1571395356", "1571395356", "1571395386", "1571395417", "1571395417", "1571395417", "1571395422", "1571396968", "1571396968", "1571396968", "1571396998", "1571397029", "1571397059", "1571397090", "1571397090", "1571397246", "1571397276"}, 1, 120),
                 genDenseInput<int32_t>({57895461}, 1, 1),
                 genDenseInput<std::string>({"1571397415"}, 1, 1),
                 genDenseInput<std::string>({"1571319156", "1571319156", "1571319156", "1571319161", "1571319161", "1571319161", "1571319204", "1571319222", "1571319222", "1571319222", "1571394459", "1571394459", "1571394459", "1571394482", "1571394494", "1571394500", "1571394507", "1571394507", "1571394507", "1571394549", "1571394563", "1571394571", "1571394578", "1571394616", "1571394620", "1571394620", "1571394620", "1571394637", "1571394637", "1571394637", "1571394637", "1571394637", "1571394637", "1571394899", "1571394998", "1571395113", "1571395113", "1571395113", "1571395132", "1571395132", "1571395132", "1571395193", "1571395205", "1571395223", "1571395223", "1571395223", "1571395279", "1571395279", "1571395279", "1571395305", "1571395313", "1571395326", "1571395339", "1571395357", "1571395396", "1571395406", "1571396950", "1571396964", "1571396988", "1571397003", "1571397003", "1571397003", "1571397043", "1571397043", "1571397043", "1571397062", "1571397076", "1571397076", "1571397076", "1571397088", "1571397213", "1571397213", "1571397213", "1571397221", "1571397221", "1571397221", "1571397232", "1571319181", "1571319211", "1571319238", "1571319238", "1571394503", "1571394503", "1571394533", "1571394533", "1571394594", "1571394624", "1571394624", "1571394686", "1571394932", "1571394932", "1571395023", "1571395084", "1571395144", "1571395174", "1571395174", "1571395205", "1571395235", "1571395265", "1571395265", "1571395265", "1571395325", "1571395325", "1571395356", "1571395356", "1571395386", "1571395417", "1571395417", "1571395417", "1571395422", "1571396968", "1571396968", "1571396968", "1571396998", "1571397029", "1571397059", "1571397090", "1571397090", "1571397246", "1571397276"}, 1, 120),
                 {3},
                 true,
                 "gap_min", 86400.0f);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffMin) {
    runDenseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<double>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int32_t>({4, 3, 5}, 3, 1),
                 genDenseInput<double>({3.0f}, 1, 1),
                 genDenseInput<double>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 {1.0, 1.0, 0.0},
                 true,
                 "gap_min", 4.0f);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffCountNeedDiscrateFalse) {
    runSingleSparseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int64_t>({4, 3, 5}, 3, 1),
                 genDenseInput<float>({3.0f}, 1, 1),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 {std::string("fg_2"), std::string("fg_1"), std::string("fg_0")},
                 true,
                 "count", 2.0f);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffMaxNeedDiscrateFalse) {
    runSingleSparseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int32_t>({4, 3, 5}, 3, 1),
                 genDenseInput<float>({10000.0f}, 1, 1),
                 genDenseInput<float>({0.0f, 1000.0f, 7000.0f, 9000.0f}, 1, 4),
                 {std::string("fg_6"), std::string("fg_4"), std::string("fg_0")},
                 true,
                 "gap_max", 86400.0f);
}

TEST_F(LookupFeatureFunctionArrayTest, testMultiSparseWithTimeDiffCountWithCut1) {
    runDenseTest(genDenseInput<int32_t>({5, 4, 4, 3}, 1, 4),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 genDenseInput<int64_t>({4, 3, 5}, 3, 1),
                 genDenseInput<float>({3.0f}, 1, 1),
                 genDenseInput<float>({0.0f, 1.0f, 2.0f, 3.0f}, 1, 4),
                 {1, 1, 0},
                 true,
                 "count", 2.0f, 1);
}

}
