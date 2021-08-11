#include <vector>
#include "fg_lite/feature/KgbMatchSemanticFeatureFunction.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {

class KgbMatchSemanticFeatureFunctionTest : public FeatureFunctionTestBase {
public:
    void genKgbMatchSemanticFeatures(const vector<FeatureInput*> &inputs,
                           const string &prefix,
                           bool match = true,
                           bool asBytes = false,
                           bool needCombo = false,
                           bool needHitRet = false,
                           bool comboRight = true)
    {
        genFeatures(inputs, [=]() {
                return new KgbMatchSemanticFeatureFunction("name", prefix, match, asBytes, needCombo, needHitRet, comboRight);
        });
    }

    void runMatchTest(FeatureInput *input1, FeatureInput *input2,
        const vector<string> &values, const vector<size_t> &offset,
        bool match = true,
        bool asBytes = false,
        bool needCombo = false,
        bool needHitRet = false) {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        genKgbMatchSemanticFeatures(inputs, "fg_", match, asBytes, needCombo, needHitRet);
        delete input1;
        delete input2;
        checkMultiSparseFeatures(values, offset);
    }

    void runMatchTest(FeatureInput *input1, FeatureInput *input2,
        FeatureInput* input3,
        const vector<string> &values, const vector<size_t> &offset,
        bool match = true,
        bool asBytes = false,
        bool needCombo = true,
        bool needHitRet = false,
        bool comboRight = true) {
        vector<FeatureInput*> inputs;
        inputs.push_back(input1);
        inputs.push_back(input2);
        inputs.push_back(input3);
        genKgbMatchSemanticFeatures(inputs, "fg_", match, asBytes, needCombo, needHitRet, comboRight);
        delete input1;
        delete input2;
        delete input3;
        checkMultiSparseFeatures(values, offset);
    }

};

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatch) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 {"fg_" + std::to_string(((1LU << 56) | 2))}, {0}
    );
}


TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatch2) {
    runMatchTest(genDenseInput<int64_t>({166260242919381816, 87722649304814285, 108103924536543660, 12961249761579518}, 1, 4),
                 genDenseInput<int64_t>({3672230621,4159476759,5237875428,5718116780,8163283661,8759395676,10543143458,11527226865,12463613676,16218329824}, 1, 10),
                 {"fg_87722649304814285", "fg_108103924536543660"}, {0}
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatch3) {
    runMatchTest(genDenseInput<std::string>({"166260242919381816", "87722649304814285", "108103924536543660", "12961249761579518"}, 1, 4),
                 genDenseInput<std::string>({"3672230621","4159476759","5237875428","5718116780","8163283661","8759395676","10543143458","11527226865","12463613676","16218329824"}, 1, 10),
                 {"fg_87722649304814285", "fg_108103924536543660"}, {0}
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatchMultiRow) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4), 2, 1, ((1LU << 32) | 2), 10}, 3, 2),
                 {"fg_" + std::to_string(((1LU << 56) | 2)), "fg_" + std::to_string(((1LU << 56) | 2))}, {0, 1, 1}
    );
}


TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermUnMatch) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 {"fg_2", "fg_3", "fg_1"}, {0},
                 false
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatchAsBytes) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), ((1LU << 56) | 4), 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((1LU << 32) | 4)}, 1, 2),
                 {"fg_" + std::to_string(((1LU << 56) | 2)) + std::to_string(((1LU << 56) | 4)) + ";;"}, {0},
                 true, true
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatchAsBytes2) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), ((2LU << 56) | 4), 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 {"fg_" + std::to_string(((1LU << 56) | 2)) + ";" + std::to_string(((2LU << 56) | 4)) + ";"}, {0},
                 true, true
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatchNeedCombo) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 genDenseInput<int64_t>({1, 14}, 1, 2),
                 {"fg_" + std::to_string(((1LU << 56) | 2)) + "_1", "fg_" + std::to_string(((1LU << 56) | 2)) + "_14"}, {0}
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatchMultiRowNeedCombo) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4), 2, 1, ((1LU << 32) | 2), 10}, 3, 2),
                 genDenseInput<int64_t>({1, 14, 4}, 3, 1),
                 {"fg_" + std::to_string(((1LU << 56) | 2)) + "_1", "fg_" + std::to_string(((1LU << 56) | 2)) + "_4"}, {0, 1, 1}
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermUnMatchNeedCombo) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 genDenseInput<int64_t>({14}, 1, 1),
                 {"fg_2_14", "fg_3_14", "fg_1_14"}, {0},
                 false
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermMatchNeedHitRet) {
    runMatchTest(genDenseInput<int64_t>({((3LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((3LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 {"fg_0"}, {0},
                 true, true, false, true
    );
}

TEST_F(KgbMatchSemanticFeatureFunctionTest, qTermItemTermUnMatchNeedComboLeft) {
    runMatchTest(genDenseInput<int64_t>({((1LU << 56) | 2), 2, 3, 1}, 1, 4),
                 genDenseInput<int64_t>({((1LU << 32) | 2), ((2LU << 32) | 4)}, 1, 2),
                 genDenseInput<int64_t>({14}, 1, 1),
                 {"fg_14_2", "fg_14_3", "fg_14_1"}, {0},
                 false, false, true, false, false
    );
}

}
