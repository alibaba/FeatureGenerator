#ifndef ISEARCH_FG_LITE_PRECLICKURBWORDFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_PRECLICKURBWORDFEATUREFUNCTION_H

#include "autil/Log.h"
#include "autil/StringUtil.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/FeatureFormatter.h"
#include "fg_lite/feature/Base64.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std;
using namespace autil;

namespace fg_lite {

static const int PRECLICK_ITEM_NUM = 10;
static const int PRECLICK_WORD_NUM = 20;
static const int LEAST_WORD_NUM = 0;
static const int MATCHED_WORD_NUM = 20;

class PreclickUrbWordFeatureFunction : public FeatureFunction
{
public:
    PreclickUrbWordFeatureFunction(const std::string &name,
                                   const std::string &prefix,
                                   bool need_decode,
                                   bool need_match,
                                   bool output_count,
                                   std::string delim_item,
                                   std::string delim_kv,
                                   bool raw_expression,
                                   bool uint64_expression);

private:
    PreclickUrbWordFeatureFunction(const PreclickUrbWordFeatureFunction &);
    PreclickUrbWordFeatureFunction& operator=(const PreclickUrbWordFeatureFunction &);

public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return _need_match ? 2 : 1;
    }

private:
    template <typename ETermType, template<typename> class StorageType>
    Features * genTopFeatureTyped(
            const FeatureInputTyped<ETermType, StorageType<ETermType>> * expressionInput) const {
        {
            if (!expressionInput) {
                return nullptr;
            }

            Base64 base64;
            std::unordered_map<std::string, int> urbMap;
            std::unordered_map<std::string, int>::iterator it;
            std::vector<std::pair<const std::string, int> *> urbPairList;
            vector<string> urbList;
            vector<string> kvList;

            MultiSparseFeatures *features = new MultiSparseFeatures(expressionInput->row());
            size_t rows = expressionInput->row();
            for (size_t i = 0; i < rows; i++) {
                features->beginDocument();

                urbMap.clear();
                int cols = expressionInput->col(i);
                int colLimit = cols < PRECLICK_ITEM_NUM ? cols : PRECLICK_ITEM_NUM;
                for (int j = 0; j < colLimit; j++) {
                    ETermType value = expressionInput->get(i, j);
                    std::string urb(value.data(), value.size());

                    if (_need_decode) {
                        std::string content;
                        if (base64.Decode(&content, urb) < 0) {
                            AUTIL_LOG(ERROR, "PreclickUrbWordFeature input %s base64 decode failed", urb.c_str());
                            return nullptr;
                        }
                        urbList = StringUtil::split(content, _delim_item);
                    } else {
                        urbList = StringUtil::split(urb, _delim_item);
                    }

                    for (auto& urbElement : urbList) {
                        if (_need_split_kv) {
                            kvList = StringUtil::split(urbElement, _delim_kv);
                            if (kvList.size() != 2) {
                                AUTIL_LOG(ERROR, "PreclickUrbWordFeature input %s is not valid kv", urbElement.c_str());
                                return nullptr;
                            }
                            urbElement = kvList[1];
                        }

                        it = urbMap.find(urbElement);
                        if (urbMap.cend() == it) {
                            urbMap.emplace(urbElement, 1);
                        } else {
                            (it->second)++;
                        }
                    }
                }

                urbPairList.clear();
                urbPairList.reserve(urbList.size());
                for (auto it = urbMap.begin(); it != urbMap.end(); ++it) {
                    if (it->second > LEAST_WORD_NUM) {
                        urbPairList.push_back(&(*it));
                    }
                }
                if (_uint64_expression) {
                    std::sort(urbPairList.begin(), urbPairList.end(), comparePairUintKey);
                } else {
                    std::sort(urbPairList.begin(), urbPairList.end(), comparePair);
                }

                size_t outputSize = PRECLICK_WORD_NUM < urbPairList.size() ? PRECLICK_WORD_NUM : urbPairList.size();
                for (size_t cnt = 0; cnt < outputSize; cnt++) {
                    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                    FeatureFormatter::fillFeatureToBuffer(urbPairList[cnt]->first, buffer);
                    features->addFeatureKey(buffer.data(), buffer.size());
                }
            }
            return features;
        }
    }

    template <typename ETermType, typename MTermType, template<typename> class StorageType>
    Features *genMatchFeatureTyped(
            const FeatureInputTyped<ETermType, StorageType<ETermType>> *expressionInput,
            const FeatureInputTyped<MTermType, StorageType<MTermType>> *matchInput) const {
        {
            if (!expressionInput || !matchInput) {
                return nullptr;
            }

            int expressionRow = expressionInput->row();
            if (expressionRow != 1) {
                AUTIL_LOG(ERROR, "urb expression input row is %d, it must be 1", expressionRow);
                return nullptr;
            }

            Base64 base64;
            std::vector<std::string> termList;
            std::vector<std::string> kvList;
            std::unordered_set<std::string> rawExpressionSet;
            std::unordered_map<std::string, int> expTermMap;
            std::unordered_map<std::string, int>::iterator it;
            std::vector<std::pair<const std::string, int> *> expTermPairList;

            // generate splitted match words, urb word-count pairs
            int expCols = expressionInput->col(0);
            int colLimit = expCols < PRECLICK_ITEM_NUM ? expCols : PRECLICK_ITEM_NUM;
            for (int j = 0; j < colLimit; j++) {
                ETermType value = expressionInput->get(0, j);
                std::string expStr(value.data(), value.size());

                // jump over split and decode
                if (_raw_expression) {
                    rawExpressionSet.insert(expStr);
                    continue;
                }

                if (_need_decode) {
                    std::string content;
                    if (base64.Decode(&content, expStr) < 0) {
                        AUTIL_LOG(ERROR, "PreclickUrbWordFeature input %s base64 decode failed", expStr.c_str());
                        return nullptr;
                    }
                    termList = StringUtil::split(content, _delim_item);
                } else {
                    termList = StringUtil::split(expStr, _delim_item);
                }

                for (auto& element : termList) {
                    if (_need_split_kv) {
                        kvList = StringUtil::split(element, _delim_kv);
                        if (kvList.size() != 2) {
                            AUTIL_LOG(ERROR, "PreclickUrbWordFeature input %s is not valid kv", element.c_str());
                            return nullptr;
                        }
                        element = kvList[1];
                    }

                    it = expTermMap.find(element);
                    if (expTermMap.cend() == it) {
                        expTermMap.emplace(element, 1);
                    } else {
                        (it->second)++;
                    }
                }
            }

            expTermPairList.reserve(termList.size());
            for (auto it = expTermMap.begin(); it != expTermMap.end(); ++it) {
                if (it->second > LEAST_WORD_NUM) {
                    expTermPairList.push_back(&(*it));
                }
            }
            if (_uint64_expression) {
                std::sort(expTermPairList.begin(), expTermPairList.end(), comparePairUintKey);
            } else {
                std::sort(expTermPairList.begin(), expTermPairList.end(), comparePair);
            }

            // for all match (ad), get matched term
            int matchRow = matchInput->row();
            MultiSparseFeatures *features = new MultiSparseFeatures(matchRow);
            std::unordered_set<std::string> matchTermSet;
            for (int i = 0; i < matchRow; i++) {
                features->beginDocument();

                int matchCol = matchInput->col(i);
                matchTermSet.clear();
                for (int j = 0; j < matchCol; j++) {
                    MTermType value = matchInput->get(i, j);
                    std::string ad(value.data(), value.size());

                    if (_need_decode) {
                        std::string content;
                        if (base64.Decode(&content, ad) < 0) {
                            AUTIL_LOG(ERROR, "PreclickUrbWordFeature input %s base64 decode failed", ad.c_str());
                            return nullptr;
                        }
                        termList = StringUtil::split(content, _delim_item);
                    } else {
                        termList = StringUtil::split(ad, _delim_item);
                    }

                    for (auto& matchElement : termList) {
                        if (_need_split_kv) {
                            kvList = StringUtil::split(matchElement, _delim_kv);
                            if (kvList.size() != 2) {
                                AUTIL_LOG(ERROR, "PreclickUrbWordFeature input %s is not valid kv", matchElement.c_str());
                                return nullptr;
                            }
                            matchElement = kvList[1];
                        }
                        matchTermSet.insert(matchElement);
                    }
                }

                int hit = 0;
                if (_raw_expression) {
                    for (auto rawExpTerm : rawExpressionSet) {
                        if (matchTermSet.find(rawExpTerm) != matchTermSet.end()) {
                            hit++;
                            if (!_output_count) {
                                FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                                FeatureFormatter::fillFeatureToBuffer(rawExpTerm, buffer);
                                features->addFeatureKey(buffer.data(), buffer.size());
                            }
                        }
                    }
                } else {
                    for (auto expTerm : expTermPairList) {
                        // if hit
                        if (matchTermSet.find(expTerm->first) != matchTermSet.end()) {
                            hit++;
                            if (!_output_count) {
                                FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                                FeatureFormatter::fillFeatureToBuffer(expTerm->first, buffer);
                                features->addFeatureKey(buffer.data(), buffer.size());
                            }
                            if (hit >= MATCHED_WORD_NUM) {
                                break;
                            }
                        }
                    }
                }

                if (_output_count) {
                    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
                    FeatureFormatter::fillFeatureToBuffer(hit, buffer);
                    features->addFeatureKey(buffer.data(), buffer.size());
                }
            }

            return features;
        }
    }

    bool _need_decode;
    bool _need_match;
    bool _output_count;
    std::string _delim_item;
    std::string _delim_kv;
    bool _need_split_kv;
    bool _raw_expression;
    bool _uint64_expression;

    static bool comparePair(const std::pair<const std::string, int> *elem1 , const std::pair<const std::string, int> *elem2) {
        return (elem1->second != elem2->second) ? (elem1->second > elem2->second) : (elem1->first < elem2->first);
    }

    static bool comparePairUintKey(const std::pair<const std::string, int> *elem1 , const std::pair<const std::string, int> *elem2) {
        uint64_t elem_key1 = 0l;
        uint64_t elem_key2 = 0l;

        if (StringUtil::strToUInt64(elem1->first.c_str(), elem_key1) && StringUtil::strToUInt64(elem2->first.c_str(), elem_key2)) {
            return (elem1->second != elem2->second) ? (elem1->second > elem2->second) : (elem_key1 < elem_key2);
        } else {
            AUTIL_LOG(ERROR, "PreclickUrbWordFeature prase expression wordcount key %s, %s to uint64 failed.", elem1->first.c_str(), elem2->first.c_str());
            return 0;
        }
    }

    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_PRECLICKURBWORDFEATUREFUNCTION_H

