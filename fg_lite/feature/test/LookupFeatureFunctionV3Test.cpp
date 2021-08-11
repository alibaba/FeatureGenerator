#include "fg_lite/feature/LookupFeatureFunctionV3.h"

#include <cstdint>
#include <inttypes.h>
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "autil/MultiValueCreator.h"
#include "autil/MurmurHash.h"
#include "autil/StringUtil.h"
#include "autil/MultiValueType.h"
#include "fg_lite/feature/LookupFeatureFunctionBTree.h"
#include "fg_lite/feature/test/FeatureFunctionTestBase.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

using namespace std;
using namespace testing;
using namespace autil;

namespace fg_lite {

class LookupFeatureFunctionV3Test : public FeatureFunctionTestBase {
public:
    void genLookupFeaturesV3(vector<FeatureInput*> inputs,
                             const uint32_t &dimension,
                             const string &prefix = "fg",
                             const Normalizer &normalizer = Normalizer(),
                             const string &combiner = "sum",
                             const vector<float> &boundaries = vector<float>()) {
        genFeatures(inputs, [=]() {
                                return new LookupFeatureFunctionV3(prefix, normalizer, combiner, dimension, boundaries);
                            });
    }
    void genLookupFeaturesBTree(vector<FeatureInput*> inputs,
                                const uint32_t &dimension,
                                const string &prefix = "fg",
                                const Normalizer &normalizer = Normalizer(),
                                const string &combiner = "sum",
                                const vector<float> &boundaries = vector<float>()) {
        genFeatures(inputs, [=]() {
                                return new LookupFeatureFunctionBTree(prefix, normalizer, combiner, dimension, boundaries);
                            });
    }

protected:
    MultiChar createOneDoc(const vector<MultiString> &doc) {
        vector<map<ConstString, float>> mp;
        for (size_t i = 0; i < doc.size(); ++i) {
            fg_lite::LookupFeatureEncoder::collectOneField(doc[i], mp);
        }

        vector<KvUnit> kvunits = KvUnit::getKvUnitVec(std::move(mp));
        std::string tmpstr;
        LookupFeatureEncoder::encodeMultiValue(kvunits, doc.size(), tmpstr);
        vector<char> vc(tmpstr.begin(), tmpstr.end());
        char *mcBuffer = MultiValueCreator::createMultiValueBuffer(vc, _pool.get());
        MultiChar mc;
        mc.init(mcBuffer);
        return mc;
    }

    MultiChar createOneDocWithBTree(const vector<MultiString> &doc) {
        vector<map<ConstString, float>> mpVec;
        // multiField
        for (size_t i = 0; i < doc.size(); ++i) {
            fg_lite::LookupFeatureEncoder::collectOneField(doc[i], mpVec);
        }
        vector<KvUnit> kvunits = KvUnit::getKvUnitVec(std::move(mpVec));
        std::string tmpstr;
        LookupFeatureEncoder::encodeMultiValueBTree(
                kvunits, doc.size(), tmpstr,
                LOOKUP_V3_KEY_HASH_0_TO_31_BIT, LOOKUP_V3_VALUE_ENCODE_AUTO);
        vector<char> vc(tmpstr.begin(), tmpstr.end());
        char *mcBuffer = MultiValueCreator::createMultiValueBuffer(vc, _pool.get());
        MultiChar mc;
        mc.init(mcBuffer);
        return mc;
    }

    template<typename FeatureT = SingleDenseFeatures, typename MultiFeatureT = MultiDenseFeatures>
    void testMatch(const vector<vector<autil::MultiString>> &docs, const vector<string> &keys,
                   const vector<float> &expected_offsets, const vector<float> &expected_values,
                   const uint32_t dim, const string &op = "sum",
                   const vector<float> &boundaries = vector<float>()) {
        vector<MultiChar> mcs;
        for (auto doc: docs) {
            mcs.push_back(createOneDoc(doc));
        }
        unique_ptr<FeatureInput> input1(genDenseInput<MultiChar>(mcs));
        unique_ptr<FeatureInput> input2(genDenseInput<string>(keys, 1, keys.size()));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genLookupFeaturesV3(inputs, dim, "", Normalizer(), op, boundaries);
        auto multiFeature = ASSERT_CAST_AND_RETURN(MultiFeatureT, _features.get());
        ASSERT_EQ(multiFeature->_offsets.size(), expected_offsets.size());
        for (uint32_t i = 0; i < multiFeature->_offsets.size(); ++i) {
            ASSERT_EQ(multiFeature->_offsets[i], expected_offsets[i]);
        }
        for (uint32_t i = 0; i < multiFeature->_featureValues.size(); ++i) {
            ASSERT_EQ(multiFeature->_featureValues[i], expected_values[i]);
        }
    }

    template<typename FeatureT = SingleDenseFeatures, typename MultiFeatureT = MultiDenseFeatures>
    void testMatchWithBTree(const vector<vector<autil::MultiString>> &docs, const vector<string> &keys,
                            const vector<float> &expected_offsets, const vector<float> &expected_values,
                            const uint32_t dim, const string &op = "sum",
                            const vector<float> &boundaries = vector<float>()) {
        vector<MultiChar> mcs;
        for (auto doc: docs) {
            mcs.push_back(createOneDocWithBTree(doc));
        }
        unique_ptr<FeatureInput> input1(genDenseInput<MultiChar>(mcs));
        // genDenseInput(values, rol_num, col_num)
        unique_ptr<FeatureInput> input2(genDenseInput<string>(keys, 1, keys.size()));
        vector<FeatureInput*> inputs;
        inputs.push_back(input1.get());
        inputs.push_back(input2.get());
        genLookupFeaturesBTree(inputs, dim, "", Normalizer(), op, boundaries);
        auto multiFeature = ASSERT_CAST_AND_RETURN(MultiFeatureT, _features.get());
        ASSERT_EQ(multiFeature->_offsets.size(), expected_offsets.size());
        for (uint32_t i = 0; i < multiFeature->_offsets.size(); ++i) {
            ASSERT_EQ(multiFeature->_offsets[i], expected_offsets[i]);
        }
        for (uint32_t i = 0; i < multiFeature->_featureValues.size(); ++i) {
            ASSERT_EQ(multiFeature->_featureValues[i], expected_values[i]);
        }
    }

    std::string make_key_need_resort(const vector<string> &all_keys, const string& leak_key) {
        uint64_t max_murmur = 0;
        uint64_t one_murmur = std::numeric_limits<uint64_t>::max();
        one_murmur = MurmurHash::MurmurHash64A(leak_key.data(), leak_key.size(), 0);
        for (const auto& key: all_keys) {
            uint64_t tmp_murmur = MurmurHash::MurmurHash64A(key.data(), key.size(), 0);
            tmp_murmur = MatchHashType<LOOKUP_V3_KEY_HASH_0_TO_31_BIT>::F(tmp_murmur);
            max_murmur = max(tmp_murmur, max_murmur);
        }
        for (uint64_t i = 0; i < std::numeric_limits<uint64_t>::max(); ++i) {
            std::string s = std::to_string(i);
            uint64_t key = MurmurHash::MurmurHash64A(s.data(), s.size(), 0);
            uint64_t hash_value = MatchHashType<LOOKUP_V3_KEY_HASH_0_TO_31_BIT>::F(key);
            if ( (key < one_murmur) && (hash_value > max_murmur)) {
                AUTIL_LOG(WARN, "target_value = %" PRIu64 "" , i);
                return std::to_string(i);
            }
        }
        return std::string();
    }
    AUTIL_LOG_DECLARE();

};

AUTIL_LOG_SETUP(fg_lite, LookupFeatureFunctionV3Test);

TEST_F(LookupFeatureFunctionV3Test, testNaN) {
    vector<vector<vector<string>>> data {
        {
            {"a:1", "b:1", "c:1", "d:1", "e:1", "f:1"},
            {"d:1", "e:1", "f:1", "g:1", "h:1", "i:1", "j:1"},
        },
    };
    vector<string> keys = {"a","i","d","j"};

    vector<float> expected_offsets = {0};
    vector<float> expected_values = {2,3};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }
    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testSimpleV3) {
    vector<vector<vector<string>>> data {
        {
            {"apple:400", "chair:130", "table:615"},
            {"apple:614545", "chair:2", "table:3"},
        },
    };
    vector<string> keys = {"apple", "chair", "table"};

    vector<float> expected_offsets = {0};
    vector<float> expected_values = {1145, 614550};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }
    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testMergeFields2) {
    vector<vector<vector<string>>> data {
        {
            {"apple:1.0", "chair:2.0", "table:3.0"},
            {"apple:10", "chair:20", "table:30"},
        },
        {
            {"apple:1.0", "chair:2.0", "table:3.0"},
            {"apple:10", "chair:20"},
        },
        {
            {"apple:1.0", "chair:2.0", "table:3.0"},
            {},
        },
        {
            {},
            {},
        },
    };
    vector<string> keys = {"apple", "table", "chair"};
    vector<float> expected_offsets = {0, 2, 4, 6};
    vector<float> expected_values = {6.0, 60, 6.0, 30, 6.0, 0, 0, 0};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }
    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_0_TO_15_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"apple:1"},
            {"apple:1"},
        },
    };
    vector<string> keys = {"apple"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {1, 1};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_16_TO_31_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"220:10", "269: -100000"},
            {"269:-1000000000"},
        },
    };
    vector<string> keys = {"220", "269"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {-99990, -1000000000};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_31_BIT);
    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_32_TO_47_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"220:1", "204:1"},
            {"269:1", "367:1"},
        },
    };
    vector<string> keys = {"220", "269", "204", "367"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {2, 2};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_31_BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_48_TO_63_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"220:1", "204:1", "146:1"},
            {"269:1", "367:1", "591:1"},
        },
    };
    vector<string> keys = {"220", "269", "204", "367", "146", "591"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {3, 3};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_31_BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_0_TO_31_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"220:1", "204:1", "146:1", "416:1"},
            {"269:1", "367:1", "591:1", "673:1"},
        },
    };
    vector<string> keys = {"220", "269", "204", "367", "146", "591", "416", "673"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {4, 4};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_31_BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testResort) {
    vector<vector<vector<string>>> data {
        {
            {"220:1", "204:1", "146:1", "416:1"},
            {"269:1", "367:1", "591:1", "673:1"},
        },
    };
    vector<string> keys = {"220", "269", "204", "367", "146", "591", "416", "673"};
    keys.emplace_back(make_key_need_resort(keys, keys[0]));
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {4, 4};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_31_BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_32_TO_63_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"220:1", "204:1", "146:1", "416:1", "99261:1"},
            {"269:1", "367:1", "591:1", "673:1", "123352:1"},
        },
    };
    vector<string> keys = {"220", "269", "204", "367", "146", "591", "416", "673",
                           "99261", "123352", "20", "62", "142", "149", "167", "176", "182", "350", "400", "474", "26", "46", "88", "108", "161", "177", "281", "315", "316", "354"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {5, 5};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_32_TO_63_BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_KEY_HASH_0_TO_63_BIT) {
    vector<vector<vector<string>>> data {
        {
            {"220:1", "204:1", "146:1", "416:1", "99261:1", "4096:1"},
            {"269:1", "367:1", "591:1", "673:1", "123352:1", "115425:1"},
        },
    };
    vector<string> keys = {"220", "269", "204", "367", "146", "591", "416", "673",
                           "99261", "123352", "4096", "115425"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {6, 6};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    char headinfo = mc[0];
    uint8_t hashType = ((headinfo + 256) % 256) & 0xfu;
    ASSERT_EQ(hashType, LOOKUP_V3_KEY_HASH_0_TO_63_BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_VALUE_ENCODE_8BIT) {
    vector<vector<vector<string>>> data {
        {
            {"apple:1"},
            {"apple:1"},
        },
    };
    vector<string> keys = {"apple"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {1, 1};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    uint8_t headinfo = (mc[0] + 256) % 256;
    uint8_t valueType = headinfo >> 4;
    ASSERT_EQ(valueType, LOOKUP_V3_VALUE_ENCODE_8BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_VALUE_ENCODE_16BIT) {
    vector<vector<vector<string>>> data {
        {
            {"a:1","b:2000"},
            {"a:1","b:2000"},
        },
    };
    vector<string> keys = {"a", "b"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {2001, 2001};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    uint8_t headinfo = (mc[0] + 256) % 256;
    uint8_t valueType = headinfo >> 4;
    ASSERT_EQ(valueType, LOOKUP_V3_VALUE_ENCODE_16BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

TEST_F(LookupFeatureFunctionV3Test, testLOOKUP_V3_VALUE_ENCODE_32BIT) {
    vector<vector<vector<string>>> data {
        {
            {"a:1","b:2000", "c:300000"},
            {"a:1","b:2000", "c:300000"},
        },
    };
    vector<string> keys = {"a", "b", "c"};
    vector<float> expected_offsets = {0};
    vector<float> expected_values = {302001, 302001};
    constexpr uint32_t dim = 2;
    autil::mem_pool::Pool pool;
    vector<vector<MultiString>> docs(data.size());
    for (size_t i = 0; i < docs.size(); ++i) {
        docs[i].resize(data[i].size());
        for (size_t j = 0; j < data[i].size(); ++j) {
            docs[i][j].init(MultiValueCreator::createMultiStringBuffer(data[i][j], &pool));
        }
    }

    // check hashtype from headinfo
    MultiChar mc = createOneDoc(docs[0]);
    uint8_t headinfo = (mc[0] + 256) % 256;
    uint8_t valueType = headinfo >> 4;
    ASSERT_EQ(valueType, LOOKUP_V3_VALUE_ENCODE_32BIT);

    testMatch         (docs, keys, expected_offsets, expected_values, dim, "sum");
    testMatchWithBTree(docs, keys, expected_offsets, expected_values, dim, "sum");
}

}
