#include "gtest/gtest.h"
#include <cmath>
#include "fg_lite/feature/FloatValueConvertor.h"
#include "autil/MultiValueCreator.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {

class FloatValueConvertorTest : public ::testing::Test {
protected:
    void checkFloatValueEqual(float val1, float val2) {
        ASSERT_TRUE(fabs(val1 - val2) < 0.00001f) << val1 << ":" << val2;
    }
};

TEST_F(FloatValueConvertorTest, testSimple) {
    int8_t int8t = 1;
    uint8_t uint8t = 2;
    int16_t int16t = 3;
    uint16_t uint16t = 4;
    int32_t int32t = 5;
    uint32_t uint32t = 6;
    int64_t int64t = 7;
    uint64_t uint64t = 8;
    bool boolValue  = true;
    bool boolValue2  = false;
    float floatValue = 3.14;
    double doubleValue = 3.14159;
    string s1 = "3.75";
    string s2 = "abc";
    string s3 = "3.75 abc";
    string s4 = "3.75 ";
    char *buf1 = MultiValueCreator::createMultiValueBuffer(s1.data(), s1.size());
    char *buf2 = MultiValueCreator::createMultiValueBuffer(s2.data(), s2.size());
    char *buf3 = MultiValueCreator::createMultiValueBuffer(s3.data(), s3.size());
    char *buf4 = MultiValueCreator::createMultiValueBuffer(s4.data(), s4.size());
                 
    MultiChar value1(buf1);
    MultiChar value2(buf2);
    MultiChar value3(buf3);
    MultiChar value4(buf4);
    
    float result;
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(int8t, result));
    checkFloatValueEqual(1.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(uint8t, result));
    checkFloatValueEqual(2.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(int16t, result));
    checkFloatValueEqual(3.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(uint16t, result));
    checkFloatValueEqual(4.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(int32t, result));
    checkFloatValueEqual(5.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(uint32t, result));
    checkFloatValueEqual(6.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(int64t, result));
    checkFloatValueEqual(7.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(uint64t, result));
    checkFloatValueEqual(8.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(boolValue, result));
    checkFloatValueEqual(1.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(boolValue2, result));
    checkFloatValueEqual(0.0, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(floatValue, result));
    checkFloatValueEqual(3.14, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(doubleValue, result));
    checkFloatValueEqual(3.14159, result);
    ASSERT_TRUE(FloatValueConvertor::convertToFloat(value1, result));
    checkFloatValueEqual(3.75, result);
    ASSERT_FALSE(FloatValueConvertor::convertToFloat(value2, result));
    ASSERT_FALSE(FloatValueConvertor::convertToFloat(value3, result));
    ASSERT_FALSE(FloatValueConvertor::convertToFloat(value4, result));
    delete [] buf1;
    delete [] buf2;
    delete [] buf3;
    delete [] buf4;
    
}

}
