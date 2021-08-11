#include "fg_lite/feature/KgbMatchSemanticFeatureFunction.h"

namespace fg_lite {

AUTIL_LOG_SETUP(fg_lite, KgbMatchSemanticFeatureFunction);

KgbMatchSemanticFeatureFunction::KgbMatchSemanticFeatureFunction(const std::string &name,
                      const std::string &prefix,
                      bool match,
                      bool asBytes,
                      bool needCombo,
                      bool needHitRet,
                      bool comboRight)
    : FeatureFunction(name, prefix)
    , _match(match)
    , _asBytes(asBytes)
    , _needCombo(needCombo)
    , _needHitRet(needHitRet)
    , _comboRight(comboRight)
    {
    }


Features *KgbMatchSemanticFeatureFunction::genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const {
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "input count invalid, expected[%d], actual[%d]",
                  (int)getInputCount(), (int)inputs.size());
        return nullptr;
    }
    FeatureInput* qTermListInput = nullptr;
    FeatureInput* iTermListInput = nullptr;
    FeatureInput* otherInput = nullptr;
    
    qTermListInput = inputs[0];
    iTermListInput = inputs[1];
    if (qTermListInput == nullptr || iTermListInput == nullptr) {
        AUTIL_LOG(ERROR, "qTermListInput(%p) or iTermListInput(%p) is nullptr!",
                qTermListInput, iTermListInput);
        return nullptr;
    }
    if (_needCombo) {
        otherInput = inputs[2];
        if (otherInput == nullptr) {
            AUTIL_LOG(ERROR, "otherInput(%p) is nullptr", otherInput);
            return nullptr;
        }
    }

    // 
    // for other ...
    //
#define FOR_EACH_ITERMTYPE(t) \
    case t : { \
        typedef InputType2Type<t>::Type IQT; \
        if (qTermListInput->storageType() == IST_DENSE) { \
            if (iTermListInput->storageType() != IST_DENSE) { \
                AUTIL_LOG(ERROR, "iTermListInput->storageType()[%d] is same as qTermListInput->storageType()[%d]", \
                        qTermListInput->storageType(), iTermListInput->storageType()); \
                return nullptr; \
            } \
            return MatchSemanticTyped(dynamic_cast<const FeatureInputTyped<QT, DenseStorage<QT>>*>(qTermListInput), \
                    dynamic_cast<const FeatureInputTyped<IQT, DenseStorage<IQT>>*>(iTermListInput), \
                    dynamic_cast<const FeatureInputTyped<IQT, DenseStorage<IQT>>*>(otherInput)); \
        } else if (qTermListInput->storageType() == IST_SPARSE_MULTI_VALUE) { \
            if (iTermListInput->storageType() != IST_SPARSE_MULTI_VALUE) { \
                AUTIL_LOG(ERROR, "iTermListInput->storageType()[%d] is same as qTermListInput->storageType()[%d]", \
                        qTermListInput->storageType(), iTermListInput->storageType()); \
                return nullptr; \
            } \
            return MatchSemanticTyped(dynamic_cast<const FeatureInputTyped<QT, MultiValueStorage<QT>>*>(qTermListInput), \
                    dynamic_cast<const FeatureInputTyped<IQT, MultiValueStorage<IQT>>*>(iTermListInput), \
                    dynamic_cast<const FeatureInputTyped<IQT, MultiValueStorage<IQT>>*>(otherInput)); \
        } else { \
            AUTIL_LOG(ERROR, "Other StorageType (%d) is not supported!", qTermListInput->storageType()); \
            return nullptr;      \
        } \
    } \
    break;
    
#define FOR_EACH_QTERMTYPE(t)                \
    case t: {                                \
        typedef InputType2Type<t>::Type QT;  \
        switch (iTermListInput->dataType()) {\
            FOR_EACH_ITERMTYPE(IT_INT64);    \
            FOR_EACH_ITERMTYPE(IT_UINT64);   \
            FOR_EACH_ITERMTYPE(IT_CSTRING);  \
            FOR_EACH_ITERMTYPE(IT_STRING);   \
        default:                             \
            AUTIL_LOG(ERROR, "not supported iTermListInput type %d", iTermListInput->dataType());\
            return nullptr;                  \
        }                                    \
    }                                        \
    break;

    switch (qTermListInput->dataType()) {
        FOR_EACH_QTERMTYPE(IT_INT64);
        FOR_EACH_QTERMTYPE(IT_UINT64);
        FOR_EACH_QTERMTYPE(IT_CSTRING);
        FOR_EACH_QTERMTYPE(IT_STRING);
    default:                             
        AUTIL_LOG(ERROR, "not supported qTermListInput type %d", iTermListInput->dataType());
        return nullptr;
    }

    return nullptr;
}



}
