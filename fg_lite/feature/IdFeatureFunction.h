#ifndef ISEARCH_FG_LITE_IDFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_IDFEATUREFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"

namespace fg_lite {

class IdFeatureFunction : public FeatureFunction
{
public:
    IdFeatureFunction(const std::string &name,
                      const std::string &prefix,
                      int32_t pruneTo,
                      const std::vector<std::string> &invalidValues);
private:
    IdFeatureFunction(const IdFeatureFunction &);
    IdFeatureFunction& operator=(const IdFeatureFunction &);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return 1;
    }
private:
    template <typename T, typename StorageTtype>
    Features *genFeatures(FeatureInput *input) const;

    template <typename TypedFeatureInput>
    void genSimpleFeatures(TypedFeatureInput *input, MultiSparseFeatures *features, int row) const;
    template <typename TypedFeatureInput>
    void genRankFeatures(TypedFeatureInput *input, MultiSparseFeatures *features, int row) const;
    template <typename TypedFeatureInput>
    void genCountFeatures(TypedFeatureInput *input, MultiSparseFeatures *features, int row) const;
    template <typename T>
    void addCountFeature(MultiSparseFeatures *features, T value, int32_t count) const;

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, bool>::type
    isInvalid(const T &inValue) const {
        for (const auto &value : _intInvalidValues) {
            if (inValue == value) {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type
    isInvalid(const T &inValue) const {
        for (const auto &value : _floatInvalidValues) {
            if (inValue == value) {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    typename std::enable_if<!std::is_floating_point<T>::value && !std::is_integral<T>::value, bool>::type
    isInvalid(const T &inValue) const {
        for (const auto &value : _invalidValues) {
            if (inValue == value) {
                return true;
            }
        }
        return false;
    }
private:
    int32_t _pruneTo;
    std::vector<std::string> _invalidValues;
    std::vector<int64_t> _intInvalidValues;
    std::vector<float> _floatInvalidValues;
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_IDFEATUREFUNCTION_H
