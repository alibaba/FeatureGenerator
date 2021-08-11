#ifndef ISEARCH_FG_LITE_KGB_MATCH_SEMATIC_H
#define ISEARCH_FG_LITE_KGB_MATCH_SEMATIC_H

#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include "autil/Log.h"
#include "autil/MultiValueType.h"
#include "autil/StringUtil.h"
#include "fg_lite/feature/FeatureFunction.h"

namespace fg_lite {

const static uint8_t CLASS_MAX_SIZE = 4;
const static uint8_t CLASS_BRAND = 3;
const static std::string kComboSplitToken = "_";

template<typename Test>
struct IsRepeated
: std::false_type {};

template<typename... Args>
struct IsRepeated<std::vector<Args...>>
: std::true_type {};

class FgLiteBytes {
public:
    template<typename T>
    inline typename std::enable_if<IsRepeated<T>::value>::type
    Append(const T& value) {
        if (0 != _buff.size()) {
            _buff.push_back(';');
        }
        //int i = 0;
        for (auto& v: value) {
            //if (i != 0) {
            //    _buff.push_back(',');
            //}
            _buff += std::to_string(v);
            //++i;
        }
    }

    const std::string& GetStr() const {
        return _buff;
    }

    template <typename T>
    FgLiteBytes& operator<<(const T& val) {
      Append(val);
      return *this;
    }

    void Clear() {
      _buff.clear();
    } 

private:
    std::string _buff;
};

inline uint8_t get_term_cls(uint64_t term) {
    return term >> 56;
}

typedef std::vector<std::vector<uint64_t>> NestedULongVector;
typedef std::vector<std::vector<uint32_t>> NestedUIntVector;
typedef std::vector<uint64_t> ULongList;
typedef std::vector<std::string> StrList;
typedef std::vector<uint64_t> TermList;
typedef std::vector<uint32_t> IdList;

class KgbMatchSemanticFeatureFunction : public FeatureFunction {
public:
    KgbMatchSemanticFeatureFunction(const std::string &name,
                      const std::string &prefix,
                      bool match = true,
                      bool asBytes = false,
                      bool needCombo = false,
                      bool needHitRet = false,
                      bool comboRight = true);
private:
    KgbMatchSemanticFeatureFunction(const KgbMatchSemanticFeatureFunction &);
    KgbMatchSemanticFeatureFunction& operator=(const KgbMatchSemanticFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;

    // 实际上两个TermType最终都必须是UINT64!
    template<typename QTermType, typename ITermType, template<typename> class StorageType>
    Features* MatchSemanticTyped(
            const FeatureInputTyped<QTermType, StorageType<QTermType>> *qTermList,
            const FeatureInputTyped<ITermType, StorageType<ITermType>> *iTermList,
            const FeatureInputTyped<ITermType, StorageType<ITermType>> *otherList = nullptr) const {
        if (qTermList == nullptr || iTermList == nullptr) {
            AUTIL_LOG(ERROR, "qTermList(%p) or iTermList(%p) is nullptr", qTermList, iTermList);
            return nullptr;
        }

        if (qTermList->row() != 1) {
            AUTIL_LOG(ERROR, "the row count of qTermList must be 1");
            return nullptr;
        }
        size_t qterms = qTermList->col(0);
        ULongList query_term_list(qterms);
        for (size_t i = 0; i < qterms; ++i) {
            uint64_t curTerm = 0;
            ConvertToUint64(qTermList->get(0, i), curTerm);
            query_term_list[i] = curTerm;
        }

        ULongList matched_term_list;
        ULongList unmatched_term_list;
        TermList matched_term_list_table[CLASS_MAX_SIZE];
        TermList unmatched_term_list_table[CLASS_MAX_SIZE];
        FgLiteBytes terms_bytes;

        size_t itermRows = iTermList->row();
        std::vector<StrList> otherIds;
        auto features = new MultiSparseFeatures(itermRows);
        NestedUIntVector item_term_list(CLASS_MAX_SIZE);
        if (_needCombo) {
            if (otherList == nullptr) {
                AUTIL_LOG(ERROR, "otherList(%p) is nullptr when _needCombo is true", otherList);
                return nullptr;
            }

            if (itermRows != otherList->row()) {
                AUTIL_LOG(ERROR, "iTermList->row(%lu) != otherList->row(%lu) when _needCombo is true",
                        itermRows, otherList->row());
                return nullptr;
            }
            
            std::string curStrId = "";
            otherIds.resize(itermRows);
            for (size_t i = 0; i < itermRows; ++i) {
                for (size_t j = 0; j < otherList->col(i); ++j) {
                    ConvertToString(otherList->get(i, j), curStrId);
                    otherIds[i].push_back(curStrId);
                }
            }
        }
        // get term list from item data:
        for (size_t i = 0; i < itermRows; ++i) {
            int hitResult = 0;
            features->beginDocument();

            item_term_list.clear();
            item_term_list.resize(CLASS_MAX_SIZE);
            for (size_t j = 0; j < iTermList->col(i); ++j) {
                uint64_t curTerm = 0; 
                ConvertToUint64(iTermList->get(i, j), curTerm);
                auto idx = GetTermIndex(curTerm);
                if (idx > CLASS_MAX_SIZE) {
                    AUTIL_LOG(ERROR, "Invalid term list value %llu, index %llu > %d", curTerm, idx, CLASS_MAX_SIZE);
                    continue;
                } else {
                    item_term_list[idx].push_back(GetTermValue(curTerm));
                }
            }
            // match ? 
            matched_term_list.clear();
            unmatched_term_list.clear();
            terms_bytes.Clear();
            for (auto& val: matched_term_list_table) {
                val.clear();
            }
            for (auto& val: unmatched_term_list_table) {
                val.clear();
            }
            for(auto val: query_term_list) {
                auto cls = get_term_cls(val);
                if (cls > CLASS_MAX_SIZE) {
                    AUTIL_LOG(ERROR, "Invalid term value %llu, cls(%d) gt %d", val, (int)cls, (int)CLASS_MAX_SIZE);
                }
                auto hit = std::binary_search(item_term_list[cls].begin(), item_term_list[cls].end(), (uint32_t)(val & 0xFFFFFFFFULL));
                if (!_asBytes) {
                    (hit ? matched_term_list : unmatched_term_list).push_back(val);
                } else {
                    hit ? matched_term_list_table[cls].push_back(val) : unmatched_term_list_table[cls].push_back(val);
                }
            }

            if (_needHitRet) {
                int match_size = matched_term_list_table[CLASS_BRAND].size();
                int unmatch_size = unmatched_term_list_table[CLASS_BRAND].size();
                hitResult = (match_size != 0) ? 0 : (unmatch_size != 0 ? 1 : 2);
                if (_needCombo) {
                    if (_comboRight) {
                        for (auto& otherId: otherIds[i]) {
                            FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                            FeatureFormatter::fillFeatureToBuffer(hitResult, buffer);
                            FeatureFormatter::fillFeatureToBuffer(kComboSplitToken, buffer);
                            FeatureFormatter::fillFeatureToBuffer(otherId, buffer);
                            features->addFeatureKey(buffer.data(), buffer.size());
                        }
                    } else {
                        for (auto& otherId: otherIds[i]) {
                            FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                            FeatureFormatter::fillFeatureToBuffer(otherId, buffer);
                            FeatureFormatter::fillFeatureToBuffer(kComboSplitToken, buffer);
                            FeatureFormatter::fillFeatureToBuffer(hitResult, buffer);
                            features->addFeatureKey(buffer.data(), buffer.size());
                        }
                    }
                } else {
                    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                    FeatureFormatter::fillFeatureToBuffer(hitResult, buffer);
                    features->addFeatureKey(buffer.data(), buffer.size());
                }
                // 不需要再走其他逻辑了！
                continue;
            }

            if (_asBytes) {
                for(auto& val: (_match ? matched_term_list_table : unmatched_term_list_table)) {
                    terms_bytes << val;
                }
                if (_needCombo) {
                    if (_comboRight) {
                        for (auto& otherId: otherIds[i]) {
                            FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                            FeatureFormatter::fillFeatureToBuffer(terms_bytes.GetStr(), buffer);
                            FeatureFormatter::fillFeatureToBuffer(kComboSplitToken, buffer);
                            FeatureFormatter::fillFeatureToBuffer(otherId, buffer);
                            features->addFeatureKey(buffer.data(), buffer.size());
                        }
                    } else {
                        for (auto& otherId: otherIds[i]) {
                            FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                            FeatureFormatter::fillFeatureToBuffer(otherId, buffer);
                            FeatureFormatter::fillFeatureToBuffer(kComboSplitToken, buffer);
                            FeatureFormatter::fillFeatureToBuffer(terms_bytes.GetStr(), buffer);
                            features->addFeatureKey(buffer.data(), buffer.size());
                        }
                    }
                } else {
                    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                    FeatureFormatter::fillFeatureToBuffer(terms_bytes.GetStr(), buffer);
                    features->addFeatureKey(buffer.data(), buffer.size());
                }
            } else {
                for (auto& val: (_match ?  matched_term_list : unmatched_term_list)) {
                    if (_needCombo) {
                        if (_comboRight) {
                            for (auto& otherId: otherIds[i]) {
                                FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                                FeatureFormatter::fillFeatureToBuffer(val, buffer);
                                FeatureFormatter::fillFeatureToBuffer(kComboSplitToken, buffer);
                                FeatureFormatter::fillFeatureToBuffer(otherId, buffer);
                                features->addFeatureKey(buffer.data(), buffer.size());
                            }
                        } else {
                            for (auto& otherId: otherIds[i]) {
                                FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                                FeatureFormatter::fillFeatureToBuffer(otherId, buffer);
                                FeatureFormatter::fillFeatureToBuffer(kComboSplitToken, buffer);
                                FeatureFormatter::fillFeatureToBuffer(val, buffer);
                                features->addFeatureKey(buffer.data(), buffer.size());
                            }
                        }
                    } else {
                        FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                        FeatureFormatter::fillFeatureToBuffer(val, buffer);
                        features->addFeatureKey(buffer.data(), buffer.size());
                    }
                }
            }
        }
        
        return features;
    }

    size_t getInputCount() const override {
        if (_needCombo) {
            return 3;
        }
        return 2;
    }

private:
    inline uint64_t GetTermIndex(uint64_t value) const {
        return value >> 32UL;
    }
    inline uint32_t GetTermValue(uint64_t value) const {
        return (uint32_t)(value & 0x00000000FFFFFFFFULL);
    }

    template<typename T>
    inline bool ConvertToUint64(const T& /*val*/, uint64_t& /*result*/) const {
        AUTIL_LOG(ERROR, "unsupported type [%s]", typeid(T).name());
        assert(false); // do not support
        return false;
    }

    template<typename T>
    inline bool ConvertToString(const T& val, std::string&  result) const {
        AUTIL_LOG(ERROR, "unsupported type [%s]", typeid(T).name());
        assert(false); // do not support
        return false;
    }

private:
    bool _match;
    bool _asBytes;
    bool _needCombo;
    bool _needHitRet;
    bool _comboRight;

    AUTIL_LOG_DECLARE();
};

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToUint64(const int64_t& val, uint64_t& result) const {
    result = (uint64_t)val;
    return true;
}

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToUint64(const uint64_t& val, uint64_t& result) const {
    result = (uint64_t)val;
    return true;
}

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToUint64(const std::string& val, uint64_t& result) const {
    return autil::StringUtil::fromString<uint64_t>(val, result);
}

template <>
inline bool KgbMatchSemanticFeatureFunction::ConvertToUint64(const autil::MultiChar &value, uint64_t& result) const {
    if (value.size() <= 0) {
        result = 0.0;
        return false;
    }
    return autil::StringUtil::fromString<uint64_t>(std::string(value.data(), value.size()), result);
}

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToString(const autil::MultiChar &value, std::string& result) const {
    result = std::string(value.data(), value.size());
    return true;
}

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToString(const std::string &value, std::string& result) const {
    result = value;
    return true;
}

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToString(const int64_t& val, std::string& result) const {
    result = std::to_string(val);
    return true;
}

template<>
inline bool KgbMatchSemanticFeatureFunction::ConvertToString(const uint64_t& val, std::string& result) const {
    result = std::to_string(val);
    return true;
}

}

#endif //ISEARCH_FG_LITE_KGB_MATCH_SEMATIC_H
