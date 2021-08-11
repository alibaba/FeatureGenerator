#include "fg_lite/feature/LookupFeatureFunctionArray.h"

#include "fg_lite/feature/FloatValueConvertor.h"
#include "fg_lite/feature/MapWrapper.h"
#include "fg_lite/feature/AnyConvert.h"
#include <algorithm>

using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, LookupFeatureFunctionArray);

LookupFeatureFunctionArray::LookupFeatureFunctionArray(const string &name,
                                                       const string &prefix,
                                                       const string &defaultLookupResult,
                                                       bool hasDefault,
                                                       bool needDiscrete,
                                                       const std::vector<float> &boundaries,
                                                       const std::string &combiner,
                                                       float timeDiff,
                                                       const std::string &combiner2,
                                                       bool needCombo,
                                                       int count1CutThreshold,
                                                       int count2CutThreshold,
                                                       bool comboRight,
                                                       bool comboSimple)
    : FeatureFunction(name, prefix),
      _defaultLookupResult(defaultLookupResult),
      _hasDefault(hasDefault),
      _needDiscrete(needDiscrete),
      _timeDiff(timeDiff),
      _combinerType(combinerConvert(combiner)),
      _combiner2Type(combinerConvert(combiner2)),
      _needCombo(needCombo),
      _count1CutThreshold(count1CutThreshold),
      _count2CutThreshold(count2CutThreshold),
      _comboRight(comboRight),
      _comboSimple(comboSimple),
      _boundaries(boundaries) {}

Features *LookupFeatureFunctionArray::genFeatures(const std::vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    if (inputs[0] == nullptr || inputs[1] == nullptr || inputs[2] == nullptr) {
        AUTIL_LOG(ERROR, "inputs[:-3][%p, %p ,%p] is nullptr?", inputs[0], inputs[1], inputs[2]);
        return nullptr;
    }

    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "input count invalid, expected[%d], actual[%d]",
                  (int)getInputCount(), (int)inputs.size());
        return nullptr;
    }

    size_t docCount = 0;
    if (!checkAndGetDocCount(inputs, docCount)) {
        AUTIL_LOG(ERROR, "input dimension invalid");
        return nullptr;
    }

    FeatureInput *mapKeyInput = inputs[0];
    FeatureInput *mapValueInput = inputs[1];
    FeatureInput *keyInput = inputs[2];
    FeatureInput *pvTime = nullptr;
    FeatureInput *urbTimes = nullptr;
    FeatureInput *map2KeyInput = nullptr;
    FeatureInput *map2ValueInput = nullptr;
    FeatureInput *urbTimes2 = nullptr;
    FeatureInput *otherInput = nullptr;
    if (_timeDiff >= 0) {
      pvTime = inputs[3];
      urbTimes = inputs[4];
      if (pvTime == nullptr || urbTimes == nullptr) {
          AUTIL_LOG(ERROR, "pvTime(%p) or urbTimes(%p) is nullptr", pvTime, urbTimes);
          return nullptr;
      }

      if (_needCombo) {
          if (_comboSimple) {
              otherInput = inputs[5];
              if (otherInput == nullptr) {
                  AUTIL_LOG(ERROR, "otherInput is nullptr when needCombo==true and comboSimple==true");
                  return nullptr;
              }
          } else {
              map2KeyInput = inputs[5];
              map2ValueInput = inputs[6];
              urbTimes2 = inputs[7];
              if (map2KeyInput == nullptr || map2ValueInput == nullptr || urbTimes2 == nullptr) {
                  AUTIL_LOG(ERROR, "map2KeyInput(%p) or map2ValueInput(%p) or urbTimes2(%p) is nullptr",
                          map2KeyInput, map2ValueInput, urbTimes2);
                  return nullptr;
              }
          }
      }
    }

    if (mapKeyInput->row() != mapValueInput->row()) {
        AUTIL_LOG(ERROR, "lookup keys values count not match, %d vs %d", int(mapKeyInput->row()), int(mapValueInput->row()));
        return nullptr;
    }

    if (urbTimes != nullptr && mapKeyInput->row() != 1 && urbTimes->row() != 1 && pvTime->row() != 1) {
        AUTIL_LOG(ERROR, "lookup keys/values shoude be 1 row!, 1 vs %d", int(mapKeyInput->row()));
        return nullptr;
    }

    auto cur_storage_type = mapKeyInput->storageType();
    if (cur_storage_type != mapValueInput->storageType()) {
        AUTIL_LOG(ERROR, "StorageType must as samp!");
        return nullptr;
    }

    if (_timeDiff >= 0.0f) {
        if (cur_storage_type != pvTime->storageType() || cur_storage_type != urbTimes->storageType()) {
            AUTIL_LOG(ERROR, "pvTime/urbTimes->StorageType must as samp as mapKeyInput!");
            return nullptr;
        }
    }

#define FOR_EACH_MAPVAL(t)                                                                                  \
    case t : {                                                                                              \
        typedef InputType2Type<t>::Type TV;                                                                 \
        if (mapValueInput->storageType() == IST_DENSE) {                                                    \
            if (keyInput->storageType() == IST_DENSE) {                                                     \
                return lookupAllTyped(dynamic_cast<const FeatureInputTyped<T, DenseStorage<T>>*>(keyInput), \
                        dynamic_cast<const FeatureInputTyped<TK, DenseStorage<TK>>*>(mapKeyInput),          \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(mapValueInput),        \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(pvTime),               \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(urbTimes),             \
                        dynamic_cast<const FeatureInputTyped<TK, DenseStorage<TK>>*>(map2KeyInput),         \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(map2ValueInput),       \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(urbTimes2),             \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(otherInput));            \
            } else if (keyInput->storageType() == IST_SPARSE_MULTI_VALUE) {                                 \
                return lookupAllTyped(dynamic_cast<const FeatureInputTyped<T, MultiValueStorage<T>>*>(keyInput), \
                        dynamic_cast<const FeatureInputTyped<TK, DenseStorage<TK>>*>(mapKeyInput),          \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(mapValueInput),        \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(pvTime),               \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(urbTimes),             \
                        dynamic_cast<const FeatureInputTyped<TK, DenseStorage<TK>>*>(map2KeyInput),         \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(map2ValueInput),      \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(urbTimes2),             \
                        dynamic_cast<const FeatureInputTyped<TV, DenseStorage<TV>>*>(otherInput));            \
            } else {                                                                                        \
                AUTIL_LOG(ERROR, "%d not supported", keyInput->storageType()); \
                return nullptr;                                         \
            }                                                                                               \
        } else if (mapValueInput->storageType() == IST_SPARSE_MULTI_VALUE) {                                \
            if (keyInput->storageType() == IST_DENSE) {                                                     \
                return lookupAllTyped(dynamic_cast<const FeatureInputTyped<T, DenseStorage<T>>*>(keyInput), \
                        dynamic_cast<const FeatureInputTyped<TK, MultiValueStorage<TK>>*>(mapKeyInput),     \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(mapValueInput),   \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(pvTime),          \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(urbTimes),        \
                        dynamic_cast<const FeatureInputTyped<TK, MultiValueStorage<TK>>*>(map2KeyInput),     \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(map2ValueInput),   \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(urbTimes2),       \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(otherInput));     \
            } else if (keyInput->storageType() == IST_SPARSE_MULTI_VALUE) {                                 \
                return lookupAllTyped(dynamic_cast<const FeatureInputTyped<T, MultiValueStorage<T>>*>(keyInput), \
                        dynamic_cast<const FeatureInputTyped<TK, MultiValueStorage<TK>>*>(mapKeyInput),     \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(mapValueInput),   \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(pvTime),          \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(urbTimes),        \
                        dynamic_cast<const FeatureInputTyped<TK, MultiValueStorage<TK>>*>(map2KeyInput),    \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(map2ValueInput),  \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(urbTimes2),       \
                        dynamic_cast<const FeatureInputTyped<TV, MultiValueStorage<TV>>*>(otherInput));     \
            } else {                                                    \
                AUTIL_LOG(ERROR, "%d not supported", keyInput->storageType()); \
                return nullptr;                                         \
            }                                                                                               \
        } else {                                                                                            \
                AUTIL_LOG(ERROR, "%d not supported", mapValueInput->storageType()); \
                return nullptr;                                         \
        }                                                                                                   \
    }                                                                                                       \
    break;                                                                                                  \

#define FOR_EACH_MAPKEY(t) \
    case t: {                                                                                  \
        typedef InputType2Type<t>::Type TK;                                                    \
        switch ((mapValueInput->dataType())) {                                                 \
            FOR_EACH_MAPVAL(IT_INT32);                                                         \
            FOR_EACH_MAPVAL(IT_INT64);                                                         \
            FOR_EACH_MAPVAL(IT_FLOAT);                                                         \
            FOR_EACH_MAPVAL(IT_DOUBLE);                                                         \
            FOR_EACH_MAPVAL(IT_CSTRING);                                                       \
            FOR_EACH_MAPVAL(IT_STRING);                                                        \
        default:                                                                               \
            AUTIL_LOG(ERROR, "not supported mapValueInput type %d", mapValueInput->dataType());\
            return nullptr;                                                                    \
        }                                                                                      \
    }                                                                                          \
    break;

#define FOR_EACH_KEYS(t)                                                                       \
    case t: {                                                                                  \
        typedef InputType2Type<t>::Type T;                                                     \
        switch ((mapKeyInput->dataType())) {                                                   \
            FOR_EACH_MAPKEY(IT_INT32);                                                         \
            FOR_EACH_MAPKEY(IT_INT64);                                                         \
            FOR_EACH_MAPKEY(IT_FLOAT);                                                         \
            FOR_EACH_MAPKEY(IT_CSTRING);                                                       \
            FOR_EACH_MAPKEY(IT_STRING);                                                        \
        default:                                                                               \
            AUTIL_LOG(ERROR, "not supported mapKeyInput type %d", mapKeyInput->dataType());    \
            return nullptr;                                                                    \
        }                                                                                      \
    }                                                                                          \
    break;

    switch ((keyInput->dataType())) {
        FOR_EACH_KEYS(IT_INT32);
        FOR_EACH_KEYS(IT_UINT32);
        FOR_EACH_KEYS(IT_INT64);
        FOR_EACH_KEYS(IT_UINT64);
        FOR_EACH_KEYS(IT_STRING);
        FOR_EACH_KEYS(IT_CSTRING);
        FOR_EACH_KEYS(IT_FLOAT);
        FOR_EACH_KEYS(IT_DOUBLE);
    default:
        AUTIL_LOG(ERROR, "not supported key type %d", keyInput->dataType());
        return nullptr;
    }

    return nullptr;
}
}
