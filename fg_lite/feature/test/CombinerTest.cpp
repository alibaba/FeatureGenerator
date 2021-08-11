#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "fg_lite/feature/Combiner.h"

using namespace std;
using namespace testing;

namespace fg_lite {

class CombinerTest : public ::testing::Test {
protected:
    template <CombinerType type>
    void doTestSingle(const vector<float> &toCollect, float finalValue) {
        Combiner<type> combiner;
        for (float v : toCollect) {
            combiner.collect(v);
        }
        ASSERT_EQ(finalValue, combiner.get()) << (int)type;
    }

    template <CombinerType type>
    void doTestMultiDimension(const vector<vector<float>> &values,
                              uint32_t dim, const vector<float> &expect) {
        float buf[dim];
        memset( buf, 0, dim*sizeof(float) );
        MultiDimensionCombiner<type> combiner(buf, dim);

        for (const auto &vec: values) {
            combiner.collect(vec.data());
        }
        EXPECT_THAT(expect, ElementsAreArray(combiner.get(), dim));
    }
};

TEST_F(CombinerTest, testSingleDimension) {
    doTestSingle<CombinerType::SUM>({1.0, 2.0, 3.0}, 6.0);
    doTestSingle<CombinerType::MEAN>({1.0, 2.0, 3.0}, 2.0);
    doTestSingle<CombinerType::MEAN>({0.0, 0.0}, 0.0);
    doTestSingle<CombinerType::MEAN>({}, 0.0);
    doTestSingle<CombinerType::MIN>({1.0, 2.0, 3.0}, 1.0);
    doTestSingle<CombinerType::MAX>({1.0, 2.0, 3.0}, 3.0);
    doTestSingle<CombinerType::COUNT>({1.0, 2.0, 3.0}, 3.0);
}

TEST_F(CombinerTest, testMultiDimension) {
    doTestMultiDimension<CombinerType::SUM>({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}, 3, {12, 15, 18});
    doTestMultiDimension<CombinerType::MEAN>({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}, 3, {4, 5, 6});
    doTestMultiDimension<CombinerType::MIN>({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}, 3, {1, 2, 3});
    doTestMultiDimension<CombinerType::MAX>({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}, 3, {7, 8, 9});
    doTestMultiDimension<CombinerType::COUNT>({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}, 3, {0, 0, 0});
}

}
