#include "gtest/gtest.h"
#include <functional>
#include "fg_lite/feature/LookupFeatureEncoder.h"
#include "fg_lite/feature/LookupFeatureBTreeClient.h"
#include "autil/StringUtil.h"
#include "autil/MurmurHash.h"
#include "autil/Log.h"
#include "autil/TimeUtility.h"

using namespace std;
using namespace testing;
using namespace autil;

const int times = 10;
uint64_t V2TimeCost  = 0;

namespace fg_lite {

class BenchMarkTest: public ::testing::Test {

private:
    std::map<int, map<int, int>> _benchMarkRec;
public:
    BenchMarkTest() {
        _benchMarkRec[2][8] = 330;
        _benchMarkRec[2][16] = 250; // avx256
        _benchMarkRec[2][32] = 185; // avx512
        _benchMarkRec[4][8] = 265;  // avx256
        _benchMarkRec[4][16] = 220; // avx512
        _benchMarkRec[4][32] = 420;
        _benchMarkRec[8][4] = 320;  // avx256
        _benchMarkRec[8][8] = 270;  // avx512
        _benchMarkRec[8][16] = 330;
    }

protected:

    template<typename KeyType, typename ValueType>
    static void getDataMapAndKeys(map<string, ValueType>& data, vector<KeyType>& keys) {
        int mapSize = 10000;
        for (int i =  0; i < mapSize; ++i) {
            string str = to_string(i);
            data[str] = i;
            KeyType finalHashKey = (KeyType)i;
            keys.push_back(finalHashKey);
        }
    }

    template<typename KeyType, typename ValueType>
    void callCalcTimeForBTree(const int blockSize, const size_t type){
        map<string, ValueType> data;
        vector<KeyType> keys;
        getDataMapAndKeys<KeyType, ValueType>(data, keys);
        calcTime(std::bind(BenchMarkTest::encodeBtree<KeyType, ValueType>, data, blockSize, type),
                 std::bind(BenchMarkTest::findBtree<KeyType, ValueType>, std::placeholders::_1, keys),
                 times, "btree", V2TimeCost);
    }

    static void calcTime(const function<string()> prepare,
                         const function<void(const string &)> &search,
                         int times, const string &name, const uint64_t expectedTime)
    {
        string data = prepare(); // prepare a string data
        AUTIL_LOG(INFO, " prepare finish, length: [%lu]", data.length());
        uint64_t start = TimeUtility::currentTime();
        AUTIL_LOG(INFO,  " === start to clock [%s] performance ===", name.c_str());
        for (int i = 0; i < times; i++) {
            search(data);
        }
        uint64_t end = TimeUtility::currentTime();
        uint64_t elapsedMs = (end - start) / 1000;
        if (name == "v2") {
            V2TimeCost = elapsedMs;
        }
        AUTIL_LOG(INFO, "[%s] time cost: %f", name.c_str(), (double) (elapsedMs) / 1000);
        ASSERT_TRUE(elapsedMs > 0);
    }

    template<typename KeyType, typename ValueType>
    static string encodeBtree(const map<string, ValueType> &data, const int blockSize, const size_t type) {
        auto hashedData = hashMap<KeyType, ValueType>(data);
        return LookupFeatureBTreeEncoder::encode<KeyType, ValueType>(hashedData, blockSize, 1, type, 2);
    }

    template<typename KeyType, typename ValueType>
    static void findBtree(const string &data, const vector<KeyType> &keys) {
        int found = 0;
        for (auto key : keys) {
            auto it = (ValueType*)branchFind<KeyType>(ConstString((char *)data.c_str(), data.length()), key, sizeof(ValueType));
            if (it != nullptr) {
                found++;
                ASSERT_EQ(key, *it);
            }
        }
        ASSERT_EQ(keys.size(), found);
    }

    static string encodeV2(const map<string, float> &data) {
        auto hashedData = hashMap<uint64_t, float>(data);
        map<uint64_t, float> myMap;
        for (auto &each : hashedData) {
            myMap[each.first] = each.second[0];
        }
        return LookupFeatureEncoder::encodeLegacyV2(myMap);
    }

    template<typename KeyType, typename ValueType>
    static void findV2(const string &data, const vector<KeyType> &keys) {
        uint64_t *keyBuffer = nullptr;
        ValueType *valueBuffer = nullptr;
        size_t itemCount = 0;
        int found = 0;
        ConstString conStrData(data.data(), data.size());
        LookupFeatureEncoder::decodeLegacyV2(conStrData.data(), conStrData.length(), &keyBuffer, &valueBuffer, &itemCount);
        uint64_t *keyBufferEnd = keyBuffer + itemCount;
        uint64_t *current = keyBuffer;
        for (auto key : keys) {
            current = lower_bound(keyBuffer, keyBufferEnd, key);
            if (*current == key) {
                found ++;
            }
        }
        ASSERT_EQ(keys.size(), found);
    }

    // static member function : only can be used inside the class, independent of self status
    template<typename KeyType, typename ValueType>
    static map<KeyType, vector<ValueType> > hashMap(const map<string, ValueType> &data) {
        map<KeyType, vector<ValueType> > doc;
        for (auto entity : data) {
            string str = entity.first;
            KeyType finalHashKey = (KeyType)stoi(str);
            doc[finalHashKey].push_back(entity.second);
        }
        return doc;
    }
protected:
    AUTIL_LOG_DECLARE();
};
AUTIL_LOG_SETUP(fg_lite, BenchMarkTest);

TEST_F(BenchMarkTest, testComparePerformance) {
    map<string, float> data;
    vector<uint64_t> keys;
    getDataMapAndKeys<uint64_t, float>(data, keys);
    calcTime(std::bind(BenchMarkTest::encodeV2, data),
             std::bind(BenchMarkTest::findV2<uint64_t, float>, std::placeholders::_1, keys),
             times, "v2", 1000); // placeholders: to be filled in calcTime()

    callCalcTimeForBTree<uint16_t, float> (8, 0);
    callCalcTimeForBTree<uint16_t, float>(16, 1);
    callCalcTimeForBTree<uint16_t, float>(32, 2);
    callCalcTimeForBTree<uint32_t, float> (8, 4);
    callCalcTimeForBTree<uint32_t, float>(16, 5);
    callCalcTimeForBTree<uint32_t, float>(32, 4);
    callCalcTimeForBTree<uint64_t, float> (4, 6);
    callCalcTimeForBTree<uint64_t, float> (8, 6);
    callCalcTimeForBTree<uint64_t, float>(16, 6);

}

} // namespace fg_lite
