#ifndef ISEARCH_FG_LITE_LOOKUPFEATURE_H
#define ISEARCH_FG_LITE_LOOKUPFEATURE_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/Normalizer.h"
#include "fg_lite/feature/Combiner.h"
#include "autil/ConstString.h"
#include <unordered_map>

namespace fg_lite {

class LookupFeatureFunction : public FeatureFunction
{
public:
    struct StringHasher {
        // get first 2 and last 6 characters as hash value.
        // keep a balance between speed and uniqueness.
        size_t operator()(const autil::ConstString& str) const {
            size_t hashValue = 0;
            char *hashValueBuf = (char*)&hashValue;
            size_t copySize;
            if ( str.size() > 6 ) {
                hashValueBuf[6] = str.data()[0];
                hashValueBuf[7] = str.data()[1];
                copySize = 6;
            } else {
                copySize = str.size();
            }
            const char *strBuf = str.data() + str.size() - copySize;
            for (size_t i = 0; i < copySize; i++) {
                hashValueBuf[i] = strBuf[i];
            }
            return hashValue;
        }
    };
    using map_alloc = autil::mem_pool::pool_allocator<std::pair<const autil::ConstString, autil::ConstString>>;
    using ConstStringMap = std::unordered_map<autil::ConstString, autil::ConstString, StringHasher,
                                              std::equal_to<autil::ConstString>, map_alloc>;
    using vec_alloc = autil::mem_pool::pool_allocator<autil::ConstString>;
    using ConstStringVec = std::vector<autil::ConstString, autil::mem_pool::pool_allocator<autil::ConstString>>;
public:
    LookupFeatureFunction(const std::string &name,
                          const std::string &prefix,
                          bool needDiscrete,
                          bool needKey,
                          const Normalizer &normalizer,
                          const std::string &combiner,
                          uint32_t dimension,
                          bool needWeighting,
                          bool isOptimized,
                          const std::vector<float> &boundaries,
                          const std::string &defaultLookupResult,
                          bool hasDefault);
private:
    LookupFeatureFunction(const LookupFeatureFunction &);
    LookupFeatureFunction& operator=(const LookupFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        if (_isOptimized) {
            return 1;
        }
        return 2;
    }
private:
    template <class FeatureWriter>
    Features *genFeatureTemplate(
            FeatureInput *itemInput,
            FeatureInput *userInput,
            FeatureFunctionContext *context) const;
    template <class FeatureWriter>
    Features *genOptimizedFeatureTemplate(
            FeatureInput *valueInput,
            FeatureFunctionContext *context) const;
    template<typename CollectFun>
    bool collectValues(FeatureInput *input,
                       size_t rowId,
                       CollectFun fun) const;
    void collectKeys(FeatureInput *key, size_t rowId,
                     FeatureFormatter::FeatureBuffer &keyBuffer,
                     ConstStringVec &keys) const;
private:
    const Normalizer _normalizer;
    const CombinerType _combinerType;
    const uint32_t _dimension;
    const bool _needDiscrete;
    const bool _needKey;
    const bool _needWeighting;
    const bool _isOptimized;
    const bool _hasDefault;
    const std::string _defaultLookupResult;
    const std::vector<float> _boundaries;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_LOOKUPFEATURE_H
