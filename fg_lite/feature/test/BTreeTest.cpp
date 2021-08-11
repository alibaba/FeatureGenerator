#include <time.h>
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "fg_lite/feature/LookupFeatureBTreeClient.h"
#include "autil/MurmurHash.h"

namespace fg_lite {

class BTreeTest : public FeatureFunctionTestBase {
public:
    static uint64_t MurmurHashFunc(int i) {
        string word = to_string(i);
        uint64_t key = autil::MurmurHash::MurmurHash64A(word.data(), word.size(), 0);
        return key;
    }
    template<typename KeyType>
    static KeyType NoHashFunc(KeyType i) {
        return i;
    }

    template<typename KeyType, typename ValueType>
    void searchHashKey(map<KeyType, vector<ValueType> >& doc, const string& data, int i, int &found, KeyType key) {
        auto it = (ValueType*)branchFind<KeyType>(ConstString((char *)data.c_str(), data.length()), key, sizeof(ValueType));
        if (nullptr != it &&doc.find(key) != doc.end()) {
            found ++;
            for (auto val : doc[key]) {
                ASSERT_EQ(val, *(it ++) );
            }
        } else {
            ASSERT_EQ(nullptr, it);
        }
    }

    template<typename KeyType, typename ValueType>
    void runBranchFind(int n, int blockSize, const function<KeyType(const int)> hashFunc, LookupFeatureV3KeyType type) {
        map<KeyType, vector<ValueType> > doc;
        for (int i = 0; i < n; ++i) {
            string str = to_string(i);
            KeyType finalHashKey = hashFunc(i);
            int value = i;
            doc[finalHashKey].push_back((KeyType)value);
        }
        string data = LookupFeatureBTreeEncoder::encode<KeyType, ValueType>(doc, blockSize, 1, type, 2);
        int found = 0;
        for (int i = 0; i < n + 100; ++i) {
            searchHashKey<KeyType, ValueType>(doc, data, i, found, hashFunc(i));
        }
        ASSERT_EQ(n, found);
    }
};

TEST_F(BTreeTest, testConstructEmpty) {
    BTree<uint64_t, float> btree(0, 3);
    auto root = btree.getRoot();
    ASSERT_EQ(0, root->children.size());
}

TEST_F(BTreeTest, testConstructHalfOneLayer) {
    BTree<uint64_t, float> btree(2, 3);
    auto root = btree.getRoot();
    ASSERT_EQ(0, root->children.size());
}

TEST_F(BTreeTest, testConstructFullLayer) {
    BTree<uint64_t, float> btree(3, 3);
    auto root = btree.getRoot();
    ASSERT_EQ(0, root->children.size());
}

TEST_F(BTreeTest, testConstructHalfMultiLayer) {
    BTree<uint64_t, float> btree(6, 3);
    auto root = btree.getRoot();
    ASSERT_EQ(1, root->children.size());
    ASSERT_EQ(0, (root->children[0])->children.size());
}

TEST_F(BTreeTest, testConstructFullMultiLayer) {
    BTree<uint64_t, float> btree1(15, 3);
    auto root = btree1.getRoot();
    ASSERT_EQ(4, root->children.size());
    ASSERT_EQ(0, root->children[1]->children.size());
    ASSERT_EQ(0, root->children[2]->children.size());
    ASSERT_EQ(0, root->children[3]->children.size());
}

TEST_F(BTreeTest, testStoreMapDataMultiLayer) {
    int n = 7;
    BTree<uint64_t, float> btree(n, 3);
    auto root = btree.getRoot();
    ASSERT_EQ(2, root->children.size());
    map<uint64_t, vector<float> > doc;
    for (int i = 1; i <= n; ++i) {
        doc[i].push_back(i);
    }
    FillMapDataBTreeVisitor<uint64_t, float> visitor1(doc, 8);
    btree.middleOrder(&visitor1);
    auto typeInfo = getHeadInfo(LOOKUP_V3_KEY_HASH_0_TO_63_BIT, LOOKUP_V3_VALUE_ENCODE_32BIT);
    StoreMapDataBTreeVisitor<uint64_t, float> visitor2(doc.size(), 8, typeInfo);
    btree.preOrder(&visitor2);
    ASSERT_EQ(1, root->children[1]->keys.size());
    ASSERT_EQ(3, root->keys.size());
}

TEST_F(BTreeTest, testEncodeEmptyDoc) {
    map<uint64_t, vector<float> > doc;
    // doc; blockSize; dimension; minHashType; ValueType;
    string output = LookupFeatureBTreeEncoder::encode<uint64_t, float>(doc, 16, 1, 6, 2);
    ASSERT_EQ(12, output.length());
}

// KeyType: uint64_t
TEST_F(BTreeTest, testMurmurHashKeyAndFind) {
    for (int n = 32; n <= 88; ++n) {
        // blockBits == 256
        runBranchFind<uint64_t, float> (n, 4, std::bind(BTreeTest::MurmurHashFunc, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
        // blockBits == 512
        runBranchFind<uint64_t, float> (n, 8, std::bind(BTreeTest::MurmurHashFunc, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
        runBranchFind<uint64_t, float> (n, 16, std::bind(BTreeTest::MurmurHashFunc, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
        runBranchFind<uint64_t, float> (n, 32, std::bind(BTreeTest::MurmurHashFunc, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_63_BIT);
    }
}

// KeyType: uint32_t
TEST_F(BTreeTest, testNoHashKeyAndFind) {
    for (int n = 32; n <= 88; ++n) {
        runBranchFind<uint32_t, float> (n, 4, std::bind(BTreeTest::NoHashFunc<uint32_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
        runBranchFind<uint32_t, float> (n, 8, std::bind(BTreeTest::NoHashFunc<uint32_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
        runBranchFind<uint32_t, float> (n, 16, std::bind(BTreeTest::NoHashFunc<uint32_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
        runBranchFind<uint32_t, float> (n, 32, std::bind(BTreeTest::NoHashFunc<uint32_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_32_TO_63_BIT);
    }
}

// KeyType: uint16_t
TEST_F(BTreeTest, test16BitKeyAndFind) {
    for (int n = 22; n <= 88; ++n) {
        runBranchFind<uint16_t, float> (n, 4, std::bind(BTreeTest::NoHashFunc<uint16_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_0_TO_15_BIT);
        runBranchFind<uint16_t, float> (n, 8, std::bind(BTreeTest::NoHashFunc<uint16_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_16_TO_31_BIT);
        runBranchFind<uint16_t, float> (n, 16, std::bind(BTreeTest::NoHashFunc<uint16_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_32_TO_47_BIT);
        runBranchFind<uint16_t, float> (n, 32, std::bind(BTreeTest::NoHashFunc<uint16_t>, std::placeholders::_1), LOOKUP_V3_KEY_HASH_48_TO_63_BIT);
    }
}
}
