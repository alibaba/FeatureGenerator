#include "fg_lite/feature/LookupFeatureFunctionArray.h"
#include "fg_lite/feature/FloatValueConvertor.h"
#include "fg_lite/feature/MapWrapper.h"
#include "fg_lite/feature/AnyConvert.h"
#include "fg_lite/feature/Normalizer.h"
#include <cmath>
#include <algorithm>

using namespace std;

namespace fg_lite {

const static std::string kFeatureSplitToken = "_";

auto TimeDiffNormal = [] (float val, float now) {
    static const float ONE_MINUTE = 60.0f;
    static const float ONE_HOUR = 3600.0f;
    if (now  - val <= 0.0001f) {
      return 0;
    }
    if (val <= ONE_MINUTE) {
        return 1;
    }
    if (val < ONE_MINUTE * 5) {
        return 2;
    }
    if (val < ONE_MINUTE * 10) {
        return 3;
    }
    if (val < ONE_MINUTE * 30) {
        return 4;
    }
    if (val < ONE_HOUR) {
        return 5;
    }
    if (val < ONE_HOUR * 6) {
        return 6;
    }
    if (val < ONE_HOUR * 12) {
        return 7;
    }
    if (val >= ONE_HOUR * 12) {
        return 8;
    }
    return 9;
};

template<typename KeyType, typename MapKeyType,
    template<typename> class KStorageType, template<typename> class StorageType, typename MapValueType>
Features *LookupFeatureFunctionArray::lookupAllTyped(
        const FeatureInputTyped<KeyType, KStorageType<KeyType>> *key,
        const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *mapKey,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *mapValue,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *pvtime,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *urbtime,
        const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *map2Key,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *map2Value,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *urbtime2,
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *otherValue) const {

    if (key == nullptr || mapKey == nullptr || mapValue == nullptr) {
        AUTIL_LOG(ERROR, "key(%p), mapKey(%p), mapValue(%p) is nullptr?", key, mapKey, mapValue);
        return nullptr;
    }
    using KeyCon = Str2ConstStr<KeyType>;
    using ActualKeyType = typename KeyCon::type;
#define CONSTRUCT_PAIRLIST2()                                                  \
    PairValueList<ActualKeyType, MapValueType> lookupPairList(pair_alloc<ActualKeyType, MapValueType>(features->getPool())); \
    double nowtm = 0.0f;                                                       \
    FloatValueConvertor::convertToDouble(pvtime->get(0, 0), nowtm);            \
    for (size_t i = 0; i < min(mapKey->row(), mapValue->row()); i++) {         \
        for (size_t j = 0; j < min(mapKey->col(i), mapValue->col(i)); j++) {   \
            if (pvtime != nullptr && urbtime != nullptr) {                     \
                double urbtm = 0.0f;                                           \
                FloatValueConvertor::convertToDouble(urbtime->get(0, j), urbtm);\
                if (nowtm - urbtm > _timeDiff) {                               \
                    continue;                                                  \
                }                                                              \
            }                                                                  \
            auto newKey = anyconvert<MapKeyType, ActualKeyType>(mapKey->get(i, j), features->getPool()); \
            lookupPairList.push_back({newKey, {mapValue->get(i, j), urbtime->get(i, j)}});               \
        }                                                                      \
    }                                                                          \
    auto cmp_function = [](const PairValueType<ActualKeyType, MapValueType>& l,\
                const PairValueType<ActualKeyType, MapValueType>& r)           \
            { return l.first < r.first; };                                     \
    std::sort(lookupPairList.begin(), lookupPairList.end(),                    \
            cmp_function);                                                     \
    PairValueList<ActualKeyType, MapValueType> lookupPairList2(pair_alloc<ActualKeyType, MapValueType>(features->getPool())); \
    for (size_t i = 0; i < min(map2Key->row(), map2Value->row()); i++) {       \
        for (size_t j = 0; j < min(map2Key->col(i), map2Value->col(i)); j++) { \
            if (pvtime != nullptr && urbtime2 != nullptr) {                    \
                double urbtm = 0.0f;                                           \
                FloatValueConvertor::convertToDouble(urbtime2->get(0, j), urbtm);\
                if (nowtm - urbtm > _timeDiff) {                               \
                    continue;                                                  \
                }                                                              \
            }                                                                  \
            auto newKey = anyconvert<MapKeyType, ActualKeyType>(map2Key->get(i, j), features->getPool()); \
            lookupPairList2.push_back({newKey, {mapValue->get(i, j), urbtime2->get(i, j)}});              \
        }                                                                      \
    }                                                                          \
    std::sort(lookupPairList2.begin(), lookupPairList2.end(),                  \
            cmp_function);                                                     \


#define CONSTRUCT_PAIRLIST()                                                   \
    PairValueList<ActualKeyType, MapValueType> lookupPairList(pair_alloc<ActualKeyType, MapValueType>(features->getPool())); \
    double nowtm = 0.0f;                                                       \
    FloatValueConvertor::convertToDouble(pvtime->get(0, 0), nowtm);            \
    for (size_t i = 0; i < min(mapKey->row(), mapValue->row()); i++) {         \
        for (size_t j = 0; j < min(mapKey->col(i), mapValue->col(i)); j++) {   \
            if (pvtime != nullptr && urbtime != nullptr) {                     \
                double urbtm = 0.0f;                                           \
                FloatValueConvertor::convertToDouble(urbtime->get(0, j), urbtm);\
                if (nowtm - urbtm > _timeDiff) {                               \
                    continue;                                                  \
                }                                                              \
            }                                                                  \
            auto newKey = anyconvert<MapKeyType, ActualKeyType>(mapKey->get(i, j), features->getPool()); \
            lookupPairList.push_back({newKey, {mapValue->get(i, j), urbtime->get(i, j)}});               \
        }                                                                      \
    }                                                                          \
    auto cmp_function = [](const PairValueType<ActualKeyType, MapValueType>& l,\
                const PairValueType<ActualKeyType, MapValueType>& r)           \
            { return l.first < r.first; };                                     \
    std::sort(lookupPairList.begin(), lookupPairList.end(),                    \
            cmp_function);                                                     \

    if (_timeDiff >= 0.0) {
        if (pvtime == nullptr || urbtime == nullptr) {
            AUTIL_LOG(ERROR, "pvtime(%p) or urbtime(%p) is nullptr when timeDiff is %f", pvtime,  urbtime, _timeDiff);
            return nullptr;
        }
        if (pvtime->row() <= 0 || pvtime->col(0) <= 0) {
            AUTIL_LOG(DEBUG, "pvtime(%p) row[%d] or col[%d] error when timeDiff is %f",
                    pvtime, int(pvtime->row()), int(pvtime->row() <= 0 ? 0 : pvtime->col(0)), _timeDiff);
            return nullptr;
        }
        if (urbtime->row() <= 0 || urbtime->col(0) <= 0) {
            AUTIL_LOG(DEBUG, "urbtime(%p) row[%d] or col[%d] error when timeDiff is %f",
                    pvtime,
                    int(urbtime->row()), int(urbtime->row() <= 0 ? 0 : urbtime->col(0)),
                    _timeDiff);
            return nullptr;
        }
        if (urbtime->row() != mapKey->row() || urbtime->col(0) != mapKey->col(0)) {
            AUTIL_LOG(ERROR, "urbtime(%p)[%d, %d] or mapKey(%p)[%d, %d]  row or col error when timeDiff is %f",
                    urbtime, int(urbtime->row()), int(urbtime->col(0)),
                    mapKey, int(mapKey->row()), int(mapKey->col(0)),
                    _timeDiff);
            return nullptr;
        }
        // kgb 定制算子: 仅支持，两个交叉都是count, 交叉方式是concat
        if (_needCombo) {
            if (!_comboSimple) {
                if (map2Key == nullptr || map2Value == nullptr || urbtime2 == nullptr) {
                    AUTIL_LOG(ERROR, "map2Key(%p) or map2Value(%p) or urbtime2(%p) is nullptr when needCombo is %d",
                            map2Key, map2Value, urbtime2, (int)_needCombo);
                    return nullptr;
                }
                if (urbtime2->row() != map2Key->row() || urbtime2->col(0) != map2Key->col(0)) {
                    AUTIL_LOG(ERROR, "urbtime2(%p)[%d, %d] or map2Key(%p)[%d, %d]  row or col error when timeDiff is %f",
                            urbtime2, int(urbtime2->row()), int(urbtime2->col(0)),
                            map2Key, int(map2Key->row()), int(map2Key->col(0)),
                            _timeDiff);
                    return nullptr;
                }
                auto features = new SingleSparseFeatures(key->row());
                CONSTRUCT_PAIRLIST2();
                if (_combiner2Type == CombinerType::NONE
                        || _combiner2Type == CombinerType::GAP_MAX
                        || _combiner2Type == CombinerType::GAP_MIN) {
                    AUTIL_LOG(ERROR, "We Just Support COUNT combiner when needCombo is True!");
                    return nullptr;
                }
                // just for count
                using LookupPairList_t = decltype(lookupPairList);
                typename LookupPairList_t::value_type tmp_kv;
                for (size_t i = 0; i < key->row(); i++) {
                    int valueRet = 0;
                    int valueRet2 = 0;
                    // 实际上暂时只需要支持key是一列的！
                    for (size_t j = 0; j < key->col(i); j++) {
                        auto toFindKey = tmp_kv;
                        toFindKey.first = KeyCon()((key->get(i, j)));
                        auto itR = std::equal_range(lookupPairList.begin(), lookupPairList.end(), toFindKey, cmp_function);
                        auto itR2 = std::equal_range(lookupPairList2.begin(), lookupPairList2.end(), toFindKey, cmp_function);
                        int matchCount = std::distance(itR.first, itR.second);
                        int matchCount2 = std::distance(itR2.first, itR2.second);
                        valueRet += matchCount;
                        valueRet2 += matchCount2;
                    }
                    if (_count1CutThreshold > 0 && valueRet > _count1CutThreshold) {
                        valueRet = _count1CutThreshold;
                    }
                    if (_count2CutThreshold > 0 && valueRet2 > _count2CutThreshold) {
                        valueRet2 = _count2CutThreshold;
                    }
                    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                    if (_comboRight) {
                        FeatureFormatter::fillFeatureToBuffer(valueRet, buffer);
                        FeatureFormatter::fillFeatureToBuffer(kFeatureSplitToken, buffer);
                        FeatureFormatter::fillFeatureToBuffer(valueRet2, buffer);
                    } else {
                        FeatureFormatter::fillFeatureToBuffer(valueRet2, buffer);
                        FeatureFormatter::fillFeatureToBuffer(kFeatureSplitToken, buffer);
                        FeatureFormatter::fillFeatureToBuffer(valueRet, buffer);
                    }
                    features->addFeatureKey(buffer.data(), buffer.size());
                }
                return features;
            } else {
                if (otherValue == nullptr) {
                    AUTIL_LOG(ERROR, "otherValue(%p) is nullptr when needCombo==true and comboSimple==true", otherValue);
                    return nullptr;
                }
                if (otherValue->row() != key->row()) {
                    AUTIL_LOG(ERROR, "otherValue->row[%lu] != key->row[%lu]", otherValue->row(), key->row());
                    return nullptr;
                }
                auto features = new MultiSparseFeatures(key->row());
                CONSTRUCT_PAIRLIST();
                if (_combiner2Type == CombinerType::NONE
                        || _combiner2Type == CombinerType::GAP_MAX
                        || _combiner2Type == CombinerType::GAP_MIN
                        || lookupPairList.empty()) {
                    AUTIL_LOG(ERROR, "We Just Support COUNT combiner when needCombo is True!");
                    return nullptr;
                }
                // just for count
                using LookupPairList_t = decltype(lookupPairList);
                typename LookupPairList_t::value_type tmp_kv;
                for (size_t i = 0; i < key->row(); i++) {
                    features->beginDocument();
                    int valueRet = 0;
                    // 实际上暂时只需要支持key是一列的！
                    for (size_t j = 0; j < key->col(i); j++) {
                        auto toFindKey = tmp_kv;
                        toFindKey.first = KeyCon()((key->get(i, j)));
                        auto itR = std::equal_range(lookupPairList.begin(), lookupPairList.end(), toFindKey, cmp_function);
                        int matchCount = std::distance(itR.first, itR.second);
                        valueRet += matchCount;
                    }
                    if (_count1CutThreshold > 0 && valueRet > _count1CutThreshold) {
                        valueRet = _count1CutThreshold;
                    }
                    size_t maxinc = otherValue->col(i);
                    if (_count2CutThreshold > 0) {
                        maxinc =  (int)maxinc > _count2CutThreshold ? _count2CutThreshold : maxinc;
                    }
                    for (size_t inc = 0; inc < maxinc; ++inc) {
                        FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                        if (_comboRight) {
                            FeatureFormatter::fillFeatureToBuffer(valueRet, buffer);
                            FeatureFormatter::fillFeatureToBuffer(kFeatureSplitToken, buffer);
                            FeatureFormatter::fillFeatureToBuffer(otherValue->get(i, inc), buffer);
                        } else {
                            FeatureFormatter::fillFeatureToBuffer(otherValue->get(i, inc), buffer);
                            FeatureFormatter::fillFeatureToBuffer(kFeatureSplitToken, buffer);
                            FeatureFormatter::fillFeatureToBuffer(valueRet, buffer);
                        }
                        features->addFeatureKey(buffer.data(), buffer.size());
                    }
                }
                return features;
            }
        }

        // kgb 定制临时算子：
        if (_combiner2Type == CombinerType::NONE) {
            auto features = new MultiSparseFeatures(key->row());
            CONSTRUCT_PAIRLIST();
            using LookupPairList_t = decltype(lookupPairList);
            typename LookupPairList_t::value_type tmp_kv;
            for (size_t i = 0; i < key->row(); i++) {
                features->beginDocument();
                for (size_t j = 0; j < key->col(i); j++) {
                    auto toFindKey = tmp_kv;
                    toFindKey.first = KeyCon()((key->get(i, j)));
                    auto itR = std::equal_range(lookupPairList.begin(), lookupPairList.end(), toFindKey, cmp_function);
                    if (std::distance(itR.first, itR.second) > 0) {
                        for (auto it = itR.first; it != itR.second; ++it) {
                            FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                            FeatureFormatter::fillFeatureToBuffer(it->second.first, buffer);
                            features->addFeatureKey(buffer.data(), buffer.size());
                        }
                        // 写默认值！
                    } else {
                        FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                        FeatureFormatter::fillFeatureToBuffer(_defaultLookupResult, buffer);
                        features->addFeatureKey(buffer.data(), buffer.size());
                    }
                }
            }
            return features;
        } else {
            if (!_needDiscrete) {
                auto features = new SingleDenseFeatures(getFeatureName(), key->row());
                CONSTRUCT_PAIRLIST();
                using LookupPairList_t = decltype(lookupPairList);
                typename LookupPairList_t::value_type tmp_kv;
                for (size_t i = 0; i < key->row(); i++) {
                    double valueRet = 0;
                    bool gap_need_use_default = false;
                    if (_combiner2Type == CombinerType::GAP_MIN) {
                        valueRet = _timeDiff;
                    } else if (_combiner2Type == CombinerType::GAP_MAX) {
                        valueRet = 0.0f;
                    }
                    // 实际上暂时只需要支持key是一列的！
                    for (size_t j = 0; j < key->col(i); j++) {
                        auto toFindKey = tmp_kv;
                        toFindKey.first = KeyCon()((key->get(i, j)));
                        auto itR = std::equal_range(lookupPairList.begin(), lookupPairList.end(), toFindKey, cmp_function);
                        int matchCount = std::distance(itR.first, itR.second);
                        if (matchCount > 0) {
                            gap_need_use_default = false;

                            // 返回命中的values的个数:
                            if (_combiner2Type == CombinerType::COUNT) {
                                valueRet += matchCount;
                            } else if (_combiner2Type == CombinerType::GAP_MIN || _combiner2Type == CombinerType::GAP_MAX) {
                                std::vector<double> tmdiffs(matchCount, 0.0f);
                                int idx = 0;
                                for (auto it = itR.first; it != itR.second; ++it, ++idx) {
                                    double urbtm = 0.0f;
                                    FloatValueConvertor::convertToDouble(it->second.second, urbtm);
                                    tmdiffs[idx] = nowtm - urbtm;
                                }
                                auto minmax_it = std::minmax_element(tmdiffs.begin(), tmdiffs.end());
                                if (_combiner2Type == CombinerType::GAP_MIN) {
                                    if (*(minmax_it.first) < valueRet) {
                                        valueRet = *(minmax_it.first);
                                    }
                                } else {
                                    if (*(minmax_it.second) > valueRet) {
                                        valueRet = *(minmax_it.second);
                                    }
                                }
                            }
                        } else {
                            if (_combiner2Type == CombinerType::GAP_MIN || _combiner2Type == CombinerType::GAP_MAX) {
                                gap_need_use_default = true;
                            }
                        }
                    }
                    // for TimeDiff ENCODE
                    if (_combiner2Type == CombinerType::GAP_MIN
                            || _combiner2Type == CombinerType::GAP_MAX) {
                        if (!gap_need_use_default) {
                            valueRet = TimeDiffNormal(valueRet, nowtm);
                        } else {
                            FloatValueConvertor::convertToDouble(_defaultLookupResult, valueRet);
                        }
                    }
                    if (_combiner2Type == CombinerType::COUNT
                            &&
                            _count1CutThreshold > 0 && valueRet > _count1CutThreshold) {
                        valueRet = _count1CutThreshold;
                    }
                    features->addFeatureValue(valueRet);
                }
                return features;
            } else {
                auto features = new SingleSparseFeatures(key->row());
                CONSTRUCT_PAIRLIST();
                using LookupPairList_t = decltype(lookupPairList);
                typename LookupPairList_t::value_type tmp_kv;
                for (size_t i = 0; i < key->row(); i++) {
                    double valueRet = 0;
                    bool gap_need_use_default = false;
                    if (_combiner2Type == CombinerType::GAP_MIN) {
                        valueRet = _timeDiff;
                    } else if (_combiner2Type == CombinerType::GAP_MAX) {
                        valueRet = 0.0f;
                    }
                    // 实际上暂时只需要支持key是一列的！
                    for (size_t j = 0; j < key->col(i); j++) {
                        auto toFindKey = tmp_kv;
                        toFindKey.first = KeyCon()((key->get(i, j)));
                        auto itR = std::equal_range(lookupPairList.begin(), lookupPairList.end(), toFindKey, cmp_function);
                        int matchCount = std::distance(itR.first, itR.second);
                        if (matchCount > 0) {
                            gap_need_use_default = false;

                            // 返回命中的values的个数:
                            if (_combiner2Type == CombinerType::COUNT) {
                                valueRet += matchCount;
                            } else if (_combiner2Type == CombinerType::GAP_MIN || _combiner2Type == CombinerType::GAP_MAX) {
                                std::vector<double> tmdiffs(matchCount, 0.0f);
                                int idx = 0;
                                for (auto it = itR.first; it != itR.second; ++it, ++idx) {
                                    double urbtm = 0.0f;
                                    FloatValueConvertor::convertToDouble(it->second.second, urbtm);
                                    tmdiffs[idx] = nowtm - urbtm;
                                }
                                auto minmax_it = std::minmax_element(tmdiffs.begin(), tmdiffs.end());
                                if (_combiner2Type == CombinerType::GAP_MIN) {
                                    if (*(minmax_it.first) < valueRet) {
                                        valueRet = *(minmax_it.first);
                                    }
                                } else {
                                    if (*(minmax_it.second) > valueRet) {
                                        valueRet = *(minmax_it.second);
                                    }
                                }
                            }
                        } else {
                            if (_combiner2Type == CombinerType::GAP_MIN || _combiner2Type == CombinerType::GAP_MAX) {
                                gap_need_use_default = true;
                            }
                        }
                    }
                    // for TimeDiff ENCODE
                    if (_combiner2Type == CombinerType::GAP_MIN
                            || _combiner2Type == CombinerType::GAP_MAX) {
                        if (!gap_need_use_default) {
                            valueRet = TimeDiffNormal(valueRet, nowtm);
                        } else {
                            FloatValueConvertor::convertToDouble(_defaultLookupResult, valueRet);
                        }
                    }
                    if (_combiner2Type == CombinerType::COUNT && _count1CutThreshold > 0 && valueRet > _count1CutThreshold) {
                        valueRet = _count1CutThreshold;
                    }
                    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                    FeatureFormatter::fillFeatureToBuffer(valueRet, buffer);
                    features->addFeatureKey(buffer.data(), buffer.size());
                }
                return features;
            }
        }
    }

#undef CONSTRUCT_PAIRLIST
#define CONSTRUCT_MAP()                                                                                             \
    MapWrapper<ActualKeyType, MapValueType> lookupMap(map_alloc<ActualKeyType, MapValueType>(features->getPool())); \
     for (size_t i = 0; i < min(mapKey->row(), mapValue->row()); i++) {                                             \
         for (size_t j = 0; j < min(mapKey->col(i), mapValue->col(i)); j++) {                                       \
             auto newKey = anyconvert<MapKeyType, ActualKeyType>(mapKey->get(i, j), features->getPool());           \
            lookupMap[newKey] = mapValue->get(i, j);                                                                \
         }                                                                                                          \
    }

    if (_needDiscrete) {
        auto features = new MultiSparseFeatures(key->row());
        CONSTRUCT_MAP();
        for (size_t i = 0; i < key->row(); i++) {
            features->beginDocument();
            for (size_t j = 0; j < key->col(i); j++) {
                auto toFindKey = KeyCon()((key->get(i, j)));
                auto it = lookupMap.find(toFindKey);
                FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                if (lookupMap.end() == it) {
                    if (!_hasDefault) {
                        continue;
                    }
                    FeatureFormatter::fillFeatureToBuffer(_defaultLookupResult, buffer);
                } else {
                    FeatureFormatter::fillFeatureToBuffer(it->second, buffer);
                }
                features->addFeatureKey(buffer.data(), buffer.size());
            }
        }
        return features;
    } else if (!_boundaries.empty()) {
        auto *features = new SingleIntegerFeatures(getFeatureName(), key->row());
        fillFeatureValue<KeyType, MapKeyType, KStorageType, StorageType, MapValueType, SingleIntegerFeatures>(key, mapKey, mapValue, features);
        return features;
    } else {
        auto *features = new SingleDenseFeatures(getFeatureName(), key->row());
        fillFeatureValue<KeyType, MapKeyType, KStorageType, StorageType, MapValueType, SingleDenseFeatures>(key, mapKey, mapValue, features);
        return features;
    }
}


template <typename KeyType, typename MapKeyType, template <typename> class KStorageType,
          template <typename> class StorageType, typename MapValueType, typename FeaturesType>
void LookupFeatureFunctionArray::fillFeatureValue(
    const FeatureInputTyped<KeyType, KStorageType<KeyType>> *key,
    const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *mapKey,
    const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *mapValue,
    FeaturesType *features) const {
    using KeyCon = Str2ConstStr<KeyType>;
    using ActualKeyType = typename KeyCon::type;
    CONSTRUCT_MAP();
    for (size_t i = 0; i < key->row(); i++) {
        float result = 0;
        for (size_t j = 0; j < key->col(i); j++) {
            auto toFindKey = KeyCon()((key->get(i, j)));
            auto it = lookupMap.find(toFindKey);
            if (lookupMap.end() != it) {
                double floatValue = 0.0f;
                FloatValueConvertor::convertToDouble(it->second, floatValue);
                result += floatValue;
            }
        }
        addFeatureMayBucketize(features, _boundaries, result);
    }
    return;
}


#define LOOKUP_ARRAY_IMPL(KeyType, MapKeyType, KStorageType, StorageType, MapValueType)      \
    template Features *LookupFeatureFunctionArray::lookupAllTyped(                           \
        const FeatureInputTyped<KeyType, KStorageType<KeyType>> *key,                        \
        const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *mapKey,                \
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *mapValue,          \
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *pvtime,            \
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *urbtime,           \
        const FeatureInputTyped<MapKeyType, StorageType<MapKeyType>> *map2Key,               \
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *map2Value,         \
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *urbtime2,          \
        const FeatureInputTyped<MapValueType, StorageType<MapValueType>> *otherValue) const;
}
