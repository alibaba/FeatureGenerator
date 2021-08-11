#include "fg_lite/feature/PreclickUrbWordFeatureFunction.h"

using namespace std;
using namespace autil;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, PreclickUrbWordFeatureFunction);

PreclickUrbWordFeatureFunction::PreclickUrbWordFeatureFunction(const std::string &name,
                                                               const std::string &prefix,
                                                               bool need_decode,
                                                               bool need_match,
                                                               bool output_count,
                                                               std::string delim_item,
                                                               std::string delim_kv,
                                                               bool raw_expression,
                                                               bool uint64_expression)
                                                               : FeatureFunction(name, prefix)
                                                               , _need_decode(need_decode)
                                                               , _need_match(need_match)
                                                               , _output_count(output_count)
                                                               , _delim_item(delim_item)
                                                               , _delim_kv(delim_kv)
                                                               , _raw_expression(raw_expression)
                                                               , _uint64_expression(uint64_expression) {
    if (_delim_item.size() == 0) {
        _delim_item = ";";
    }
    _need_split_kv = (_delim_kv.size() != 0);
};

Features* PreclickUrbWordFeatureFunction::genFeatures(const std::vector<fg_lite::FeatureInput *> &inputs,
                                                      fg_lite::FeatureFunctionContext *context) const {
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "PreclickUrbWordFeature[%s] inputs size not equal 1(for top) or 2(for match)", getFeatureName().c_str());
        return nullptr;
    }
    FeatureInput *expressionInput = inputs[0];

    if (_need_match) {
        FeatureInput *matchInput = inputs[1];

#define FOR_MATCH_ITEMTYPE(T) \
    case T: { \
        typedef InputType2Type<T>::Type MT; \
        if (expressionInput->storageType() == IST_DENSE) { \
            if (matchInput->storageType() == IST_DENSE) { \
                return genMatchFeatureTyped( \
                        dynamic_cast<const FeatureInputTyped<ET, DenseStorage<ET>>*>(expressionInput), \
                        dynamic_cast<const FeatureInputTyped<MT, DenseStorage<MT>>*>(matchInput)); \
            } else{ \
                AUTIL_LOG(ERROR, "PreclickUrbWordFeature expression and match input storage type not match"); \
                return nullptr; \
            } \
            return nullptr; \
        } else if (expressionInput->storageType() == IST_SPARSE_MULTI_VALUE) { \
            if (matchInput->storageType() == IST_DENSE) { \
                AUTIL_LOG(ERROR, "PreclickUrbWordFeature unsupport match input storage type %d", matchInput->storageType()); \
                return nullptr; \
            } else if (matchInput->storageType() == IST_SPARSE_MULTI_VALUE) { \
                return genMatchFeatureTyped( \
                        dynamic_cast<const FeatureInputTyped<ET, MultiValueStorage<ET>>*>(expressionInput), \
                        dynamic_cast<const FeatureInputTyped<MT, MultiValueStorage<MT>>*>(matchInput)); \
            } else{ \
                AUTIL_LOG(ERROR, "PreclickUrbWordFeature expression and match input storage type not match"); \
                return nullptr; \
            } \
        } else { \
            if (matchInput->storageType() == IST_DENSE) { \
                AUTIL_LOG(ERROR, "PreclickUrbWordFeature unsupport match input storage type %d", matchInput->storageType()); \
                return nullptr; \
            } else if (matchInput->storageType() == IST_SPARSE_MULTI_VALUE) { \
                AUTIL_LOG(ERROR, "PreclickUrbWordFeature expression and match input storage type not match"); \
                return nullptr; \
            } else{ \
                return genMatchFeatureTyped( \
                        dynamic_cast<const FeatureInputTyped<ET, ValueOffsetStorage<ET>>*>(expressionInput), \
                        dynamic_cast<const FeatureInputTyped<MT, ValueOffsetStorage<MT>>*>(matchInput)); \
            } \
        } \
    } \
    break;

#define FOR_EXP_ITEMTYPE(T) \
    case T: { \
        typedef InputType2Type<T>::Type ET; \
        switch (matchInput->dataType()) { \
            FOR_MATCH_ITEMTYPE(IT_STRING); \
            FOR_MATCH_ITEMTYPE(IT_CSTRING); \
            default: \
                AUTIL_LOG(ERROR, "PreclickUrbWordFeature unsupport match input type %d", matchInput->dataType()); \
                return nullptr; \
        } \
    } \
    break;

        switch (expressionInput->dataType()) {
            FOR_EXP_ITEMTYPE(IT_STRING);
            FOR_EXP_ITEMTYPE(IT_CSTRING);
        default:
            AUTIL_LOG(ERROR, "PreclickUrbWordFeature unsupport expression input type %d", expressionInput->dataType()); 
            return nullptr;
        }
#undef FOR_EXP_ITEMTYPE
#undef FOR_MATCH_ITEMTYPE

    } else {
#define FOR_TOP_ITEMTYPE(T) \
    case T:{  \
        typedef InputType2Type<T>::Type Type; \
        if (expressionInput->storageType() == IST_DENSE) { \
            return genTopFeatureTyped( \
                    dynamic_cast<const FeatureInputTyped<Type, DenseStorage<Type>>*>(expressionInput)); \
        } else if (expressionInput->storageType() == IST_SPARSE_MULTI_VALUE) { \
            return genTopFeatureTyped( \
                    dynamic_cast<const FeatureInputTyped<Type, MultiValueStorage<Type>>*>(expressionInput)); \
        } else { \
            return genTopFeatureTyped( \
                    dynamic_cast<const FeatureInputTyped<Type, ValueOffsetStorage<Type>>*>(expressionInput)); \
        } \
    } \
    break;

        switch (expressionInput->dataType()) {
            FOR_TOP_ITEMTYPE(IT_STRING);
            FOR_TOP_ITEMTYPE(IT_CSTRING);
        default:
            AUTIL_LOG(ERROR, "PreclickUrbWordFeature unsupport input type %d", expressionInput->dataType());
            return nullptr;
        }
    }
#undef FOR_TOP_ITEMTYPE
}

}
