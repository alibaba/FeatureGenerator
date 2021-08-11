#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public
#include "fg_lite/feature/LookupFeatureSparseEncoder.h"

using namespace std;
using namespace testing;

namespace fg_lite {

/////////////////////////////////////////
class SparseFormatterTest : public ::testing::Test {
protected:
    template <typename KeyType, typename ValueType>
    void doTestConstruct(uint32_t keyOffset, uint32_t offsetOffset, uint32_t valueOffset) {
        char buffer[1024];
        *(uint32_t *)buffer = 2;
        SparseFormatter<KeyType, ValueType> formatter(buffer, 4);
        ASSERT_EQ(2, formatter._keyNum);
        ASSERT_EQ(buffer+keyOffset, (char *)formatter._keys);
        ASSERT_EQ(buffer+offsetOffset, (char *)formatter._offsets);
        ASSERT_EQ(buffer+valueOffset, (char *)formatter._values);
    }

    template <typename KeyType, typename ValueType>
    void doTestGetEncodedLength(uint32_t keyNum, uint32_t valueNum,
                                uint32_t dimension, uint32_t expectedLength)
    {
        using TypedSparseFormatter = SparseFormatter<KeyType, ValueType>;
        ASSERT_EQ(expectedLength, TypedSparseFormatter::getEncodedLength(keyNum, valueNum, dimension));
    }
};

TEST_F(SparseFormatterTest, testConstruct) {
    doTestConstruct<uint16_t, uint8_t>(4, 8, 16);
    doTestConstruct<uint16_t, uint16_t>(4, 8, 16);
    doTestConstruct<uint16_t, float>(4, 8, 16);

    doTestConstruct<uint32_t, uint8_t>(4, 12, 20);
    doTestConstruct<uint32_t, uint16_t>(4, 12, 20);
    doTestConstruct<uint32_t, float>(4, 12, 20);

    doTestConstruct<uint64_t, uint8_t>(4, 20, 28);
    doTestConstruct<uint64_t, uint16_t>(4, 20, 28);
    doTestConstruct<uint64_t, float>(4, 20, 28);
}

TEST_F(SparseFormatterTest, testGetEncodedLength) {
    doTestGetEncodedLength<uint64_t, float>(0, 0, 4, 4);
    doTestGetEncodedLength<uint64_t, float>(2, 0, 4, 36);
    doTestGetEncodedLength<uint64_t, float>(2, 5, 4, 56);

    doTestGetEncodedLength<uint32_t, float>(0, 0, 4, 4);
    doTestGetEncodedLength<uint32_t, float>(2, 0, 4, 28);
    doTestGetEncodedLength<uint32_t, float>(2, 5, 4, 48);

    doTestGetEncodedLength<uint16_t, float>(0, 0, 4, 4);
    doTestGetEncodedLength<uint16_t, float>(2, 0, 4, 24);
    doTestGetEncodedLength<uint16_t, float>(2, 5, 4, 44);
}

TEST_F(SparseFormatterTest, testEncodeAndDecodeValue) {
    char buffer[1024] = {0};
    *(uint32_t *)buffer = 3;
    SparseFormatter<uint64_t, float> formatter(buffer, 4);
    uint8_t *valueAddr = formatter.getValues();

    float results[4];
    size_t bytes = formatter.encodeValue(valueAddr, {1.0, 0.0, 2.0, 0.0, 1.1});
    ASSERT_EQ(12, bytes);
    formatter.decodeValue(valueAddr, results, 4);
    EXPECT_THAT(results, ElementsAre(1.0, 0.0, 2.0, 0.0));

    bytes += formatter.encodeValue(valueAddr + bytes, {0.0, 0.0, 0.0, 0.0, 0.0});
    ASSERT_EQ(16, bytes);
    formatter.decodeValue(valueAddr + 12, results, 4);
    EXPECT_THAT(results, ElementsAre(0.0, 0.0, 0.0, 0.0));

    bytes += formatter.encodeValue(valueAddr + bytes, {3.0, 1.0, 2.0});
    ASSERT_EQ(32, bytes);
    formatter.decodeValue(valueAddr + 16, results, 4);
    EXPECT_THAT(results, ElementsAre(3.0, 1.0, 2.0, 0.0));
}

/////////////////////////////////////////
class SparseEncoderTest : public ::testing::Test {
protected:
    template<typename KeyType, typename ValueType>
    void doTestConstruct(uint32_t keyNum, uint32_t valueNum, uint32_t dimension, size_t ret) {
        SparseEncoder<KeyType, ValueType> encoder(keyNum, valueNum, dimension);
        ASSERT_EQ(encoder._encodeBuffer.size(), ret);
        ASSERT_EQ(encoder._encodedValueSize, 0);
    }

    template<typename KeyType, typename ValueType>
    void doTestEncodeDecode(uint32_t keyNum, uint32_t valueNum, uint32_t dimension,
                            const map<KeyType, vector<float>> &inputData,
                            const vector<KeyType> &notFounds) {
        SparseEncoder<KeyType, ValueType> encoder(keyNum, valueNum, dimension);
        for (const auto& keyValuesPair: inputData) {
            encoder.encode(keyValuesPair.first,keyValuesPair.second);
        }

        SparseDecoder<KeyType, ValueType> decoder(encoder._encodeBuffer.data(), dimension);

        float buf[dimension];
        for (const auto &keyValuesPair: inputData) {
            ASSERT_TRUE(decoder.find(keyValuesPair.first, buf, dimension));
            EXPECT_THAT(keyValuesPair.second, ElementsAreArray(buf, dimension));
        }
        for (auto key : notFounds) {
            ASSERT_FALSE(decoder.find(key, buf, dimension));
        }
    }
};

TEST_F(SparseEncoderTest, testConstruct) {
    doTestConstruct<uint16_t, uint8_t>(2, 5, 4, 29);
    doTestConstruct<uint16_t, uint16_t>(2, 5, 4, 34);
    doTestConstruct<uint16_t, float>(2, 5, 4, 44);
    doTestConstruct<uint32_t, uint8_t>(2, 5, 4, 33);
    doTestConstruct<uint32_t, uint16_t>(2, 5, 4, 38);
    doTestConstruct<uint32_t, float>(2, 5, 4, 48);
    doTestConstruct<uint64_t, uint8_t>(2, 5, 4, 41);
    doTestConstruct<uint64_t, uint16_t>(2, 5, 4, 46);
    doTestConstruct<uint64_t, float>(2, 5, 4, 56);
}

TEST_F(SparseEncoderTest, testEncodeDecode) {
    doTestEncodeDecode<uint16_t, uint8_t>(2, 5, 4,
            {
                {123, {0, 11, 22, 33}},
                {234, {0, 44, 55, 0}}
            },
            {789});
    doTestEncodeDecode<uint32_t, uint8_t>(2, 5, 4,
            {
                {123, {0, 11, 22, 33}},
                {234, {0, 44, 55, 0}}
            },
            {789});
    doTestEncodeDecode<uint64_t, uint8_t>(2, 5, 4,
            {
                {123, {0, 11, 22, 33}},
                {234, {0, 44, 55, 0}}
            },
            {789});

    doTestEncodeDecode<uint16_t, uint16_t>(2, 5, 4,
            {
                {123, {0, 11, 22, 33}},
                {234, {0, 44, 55, 0}}
            },
            {789});
    doTestEncodeDecode<uint32_t, uint16_t>(2, 5, 4,
            {
                {123, {0, 11, 22, 33}},
                {234, {0, 44, 55, 0}}
            },
            {789});
    doTestEncodeDecode<uint64_t, uint16_t>(2, 5, 4,
            {
                {123, {0, 11, 22, 33}},
                {234, {0, 44, 55, 0}}
            },
            {789});

    doTestEncodeDecode<uint16_t, float>(2, 5, 4,
            {
                {123, {0, 1.1, 2.2, 3.3}},
                {234, {0, 4.4, 5.5, 0}}
            },
            {789});
    doTestEncodeDecode<uint32_t, float>(2, 5, 4,
            {
                {123, {0, 1.1, 2.2, 3.3}},
                {234, {0, 4.4, 5.5, 0}}
            },
            {789});
    doTestEncodeDecode<uint64_t, float>(2, 5, 4,
            {
                {123, {0, 1.1, 2.2, 3.3}},
                {234, {0, 4.4, 5.5, 0}}
            },
            {789});
}

}
