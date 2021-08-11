#include "gtest/gtest.h"
#include "gmock/gmock.h"
#define private public
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "fg_lite/feature/FeatureFunctionCreator.h"
#include "fg_lite/feature/FeatureConfig.h"
#include "fg_lite/feature/MatchFeatureFunction.h"
#include "fg_lite/feature/IdFeatureFunction.h"
#include "fg_lite/feature/ComboFeatureFunction.h"
#include "fg_lite/feature/RawFeatureFunction.h"
#include "fg_lite/feature/GBDTFeatureFunction.h"
#include "fg_lite/feature/LookupFeatureFunction.h"
#include "fg_lite/feature/LookupFeatureFunctionV2.h"
#include "fg_lite/feature/LookupFeatureFunctionArray.h"
#include "fg_lite/feature/OverLapFeatureFunction.h"

using namespace std;
using namespace testing;

namespace fg_lite {

class FeatureFunctionCreatorTest : public ::testing::Test {};

TEST_F(FeatureFunctionCreatorTest, testCreateMatchFeatureFunction) {
    MatchFeatureConfig featureConfig("brand_hit");
    featureConfig.categoryExpression = "ALL";
    featureConfig.userExpression = "user:user";
    featureConfig.itemExpression = "item:item";
    featureConfig.matchType = "hit";
    featureConfig.needDiscrete = true;
    featureConfig.needWeighting = true;

    FeatureFunction *featFunc = FeatureFunctionCreator::createFeatureFunction(
            &featureConfig);
    MatchFeatureFunction *matchFunc = dynamic_cast<MatchFeatureFunction *>(featFunc);
    ASSERT_TRUE(matchFunc);
    ASSERT_TRUE(matchFunc->_wildCardCategory);
    delete matchFunc;

    featureConfig.categoryExpression = "category";
    featureConfig.itemExpression = "item";
    featFunc = FeatureFunctionCreator::createFeatureFunction(&featureConfig);
    matchFunc = dynamic_cast<MatchFeatureFunction *>(featFunc);
    ASSERT_TRUE(matchFunc);
    ASSERT_TRUE(matchFunc->_matcher->needWeighting());
    ASSERT_FALSE(matchFunc->_wildCardCategory);
    delete matchFunc;
}

TEST_F(FeatureFunctionCreatorTest, testCreateLookupFeature) {
    LookupFeatureConfig featConfig;
    featConfig.keyExpression = "user:usersex";
    featConfig.mapExpression = "attr2";
    FeatureFunction *fun = FeatureFunctionCreator::createFeatureFunction(
            &featConfig);
    LookupFeatureFunction *funTyped = ASSERT_CAST_AND_RETURN(LookupFeatureFunction, fun);
    ASSERT_TRUE(funTyped != nullptr);
    ASSERT_FALSE(funTyped->_needWeighting);
    delete fun;
}

TEST_F(FeatureFunctionCreatorTest, testCreateLookupFeatureWithWeighting) {
    LookupFeatureConfig featConfig;
    featConfig.keyExpression = "user:usersex";
    featConfig.mapExpression = "attr2";
    featConfig.needWeighting = true;
    FeatureFunction *fun = FeatureFunctionCreator::createFeatureFunction(
            &featConfig);
    LookupFeatureFunction *funTyped = ASSERT_CAST_AND_RETURN(LookupFeatureFunction, fun);
    ASSERT_TRUE(funTyped != nullptr);
    ASSERT_TRUE(funTyped->_needWeighting);
    delete fun;
}

TEST_F(FeatureFunctionCreatorTest, testCreateLookupFeatureV2) {
    LookupFeatureConfigV2 featConfig;
    featConfig.keyExpression = "user:usersex";
    featConfig.mapExpression = "attr2";
    featConfig.needDiscrete = false;
    FeatureFunction *fun = FeatureFunctionCreator::createFeatureFunction(&featConfig);
    LookupFeatureFunctionV2 *funTyped = ASSERT_CAST_AND_RETURN(LookupFeatureFunctionV2, fun);
    ASSERT_TRUE(funTyped != nullptr);
    delete fun;
}

TEST_F(FeatureFunctionCreatorTest, testCreateLookupFeatureArray) {
    LookupFeatureConfig featConfig;
    featConfig.mapKeysExpression = "user:user:key";
    featConfig.mapValuesExpression = "user:map";
    FeatureFunction *fun = FeatureFunctionCreator::createFeatureFunction(&featConfig);
    auto funTyped = ASSERT_CAST_AND_RETURN(LookupFeatureFunctionArray, fun);
    ASSERT_TRUE(funTyped);
    delete fun;

}

TEST_F(FeatureFunctionCreatorTest, testCreateLookupFeatureArrayBad) {
    LookupFeatureConfig featConfig;
    featConfig.mapValuesExpression = "user:map";
    FeatureFunction *fun =
        FeatureFunctionCreator::createFeatureFunction(&featConfig);
    auto funTyped = ASSERT_CAST_AND_RETURN(LookupFeatureFunction, fun);
    ASSERT_TRUE(funTyped);
    delete fun;
}

}
