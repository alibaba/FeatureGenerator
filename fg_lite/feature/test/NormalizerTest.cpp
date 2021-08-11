#include "gtest/gtest.h"
#include "fg_lite/feature/Normalizer.h"

using namespace std;
using namespace testing;

namespace fg_lite {

class NormalizerTest : public ::testing::Test {
};

TEST_F(NormalizerTest, testParse) {
    Normalizer normalizer;

    ASSERT_FLOAT_EQ(101.0, normalizer.normalize(101.0));
    ASSERT_TRUE(normalizer.parse(""));
    ASSERT_FLOAT_EQ(202.0, normalizer.normalize(202.0));
    ASSERT_TRUE(normalizer.parse("method=minmax, min=2.1, max=12.1"));
    ASSERT_FLOAT_EQ(2.1, normalizer._param1);
    ASSERT_FLOAT_EQ(12.1, normalizer._param2);
    ASSERT_FLOAT_EQ(0.3, normalizer.normalize(5.1));
    ASSERT_TRUE(normalizer.parse("method=zscore, mean=0.5, standard_deviation=10.0"));
    ASSERT_FLOAT_EQ(0.5, normalizer._param1);
    ASSERT_FLOAT_EQ(10.0, normalizer._param2);
    ASSERT_FLOAT_EQ(0.45, normalizer.normalize(5.0));
    ASSERT_TRUE(normalizer.parse("method=log10"));
    ASSERT_FLOAT_EQ(1.0, normalizer.normalize(10.0));
}

TEST_F(NormalizerTest, testParseFail) {
    Normalizer normalizer;
    ASSERT_FALSE(normalizer.parse("method="));

    ASSERT_FALSE(normalizer.parse("method=minmax"));
    ASSERT_FALSE(normalizer.parse("method=zscore"));

    ASSERT_FALSE(normalizer.parse("method=minmax, min=2.1, max=2.1"));
    ASSERT_FALSE(normalizer.parse("method=minmax, min=3.1, max=2.1"));
    ASSERT_FALSE(normalizer.parse("method=zscore, mean=0.5, standard_deviation=0.0"));
}

TEST_F(NormalizerTest, testMinMax) {
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=minmax, min=2.1, max=12.1"));
    ASSERT_FLOAT_EQ(-0.1, normalizer.normalize(1.1));
    ASSERT_FLOAT_EQ(3.0, normalizer.normalize(32.1));
}

TEST_F(NormalizerTest, testZscore) {
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=zscore, mean=0.0, standard_deviation=10.0"));
    ASSERT_FLOAT_EQ(0.0, normalizer.normalize(0.0));
    ASSERT_FLOAT_EQ(-0.1, normalizer.normalize(-1.0));
}

TEST_F(NormalizerTest, testBoundRegular) {
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=bound_regular, mean=0.0, standard_deviation=10.0, "
                                 "y_add=0.0, y_max=-10000.0, y_min=10000.0, log=false"));
    ASSERT_FLOAT_EQ(0.0, normalizer.normalize(0.0));
    ASSERT_FLOAT_EQ(-0.1, normalizer.normalize(-1.0));
    ASSERT_TRUE(normalizer.parse("method=bound_regular,mean=7.341363,standard_deviation=2.361654,"
                                 "y_add=0.010000,y_min=1000000000.000000,y_max=0.000000,log=true"));
}

TEST_F(NormalizerTest, testLog10) {
    Normalizer normalizer;
    ASSERT_TRUE(normalizer.parse("method=log10"));
    ASSERT_FLOAT_EQ(0.0, normalizer.normalize(-1));
    ASSERT_FLOAT_EQ(0.0, normalizer.normalize(0));
    ASSERT_FLOAT_EQ(0.0, normalizer.normalize(1));
    ASSERT_FLOAT_EQ(0.0, normalizer.normalize(0.1));

    ASSERT_TRUE(normalizer.parse("method=log10,threshold=1e-10,default=-10"));
    ASSERT_FLOAT_EQ(-10, normalizer.normalize(-1));
    ASSERT_FLOAT_EQ(-10, normalizer.normalize(0));
    ASSERT_FLOAT_EQ(1.0, normalizer.normalize(10));
    ASSERT_FLOAT_EQ(-1.0, normalizer.normalize(0.1));

    ASSERT_FALSE(normalizer.parse("method=log10,a=b"));
    ASSERT_FALSE(normalizer.parse("method=log10,a=b,c=d"));
    ASSERT_FALSE(normalizer.parse("method=log10,threshold=-0.1"));
}

}
