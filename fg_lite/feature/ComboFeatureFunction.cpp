#include "fg_lite/feature/ComboFeatureFunction.h"
#include "fg_lite/feature/FeatureFormatter.h"

using namespace autil;
using namespace std;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, ComboFeatureFunction);

ComboFeatureFunction::ComboFeatureFunction(const string &name, const string &prefix,
        const std::vector<bool> &pruneRight,
        const std::vector<int> &pruneLimit,
        size_t inputCount,
        bool needSort)
    : FeatureFunction(name, prefix)
    , _inputCount(inputCount)
    , _pruneRight(pruneRight)
    , _pruneLimit(pruneLimit)
    , _needSort(needSort)
{
    if (_pruneLimit.size() < inputCount) {
        std::fill_n(std::back_inserter(_pruneLimit), inputCount - _pruneLimit.size(), std::numeric_limits<int>::max());
    }
    if (_pruneRight.size() < inputCount) {
        std::fill_n(std::back_inserter(_pruneRight), inputCount - _pruneRight.size(), true);
    }
}

static bool isAllSingleValueInput(const vector<FeatureInput*> &inputs) {
    for (size_t i = 0; i < inputs.size(); i++) {
        if (inputs[i]->storageType() != IST_DENSE) {
            return false;
        }
        for (size_t r = 0; r < inputs[i]->row(); r++) {
            if (inputs[i]->col(r) != 1) {
                return false;
            }
        }
    }
    return true;
}

Features* ComboFeatureFunction::genFeatures(
        const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "input count invalid, expected[%d], actual[%d]",
                  (int)getInputCount(), (int)inputs.size());
        return nullptr;
    }
    size_t docCount;
    if (!checkAndGetDocCount(inputs, docCount)) {
        AUTIL_LOG(ERROR, "feature[%s] docCounts not equal 1 or not one user input",
                  getFeatureName().c_str());
        return nullptr;
    }
    bool isAllSingle = isAllSingleValueInput(inputs);
    MultiSparseFeatures *features = new MultiSparseFeatures(docCount);
    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(features->getPool());
    vector<FeatureFormatter::FeatureBuffer> bufferVec;
    for (size_t i = 0; i < docCount; i++) {
        features->beginDocument();
#define DO_GEN_FEATURE(Combo)                                           \
        if (isAllSingle) {                                              \
            genFeatureFast(inputs, i, features, combo);          \
        } else {                                                        \
            genFeatureNormal(inputs, i, 0,  features, combo);    \
        }

        if (_needSort) {
            SortedCombo combo(buffer);
            DO_GEN_FEATURE(SortedCombo)
        } else {
            NormalCombo combo(buffer);
            DO_GEN_FEATURE(NormalCombo)
        }
#undef DO_GEN_FEATURE
    }
    return features;
}

template<typename Combo>
void ComboFeatureFunction::genFeatureFast(
        const std::vector<FeatureInput*> &inputs,
        size_t id,
        MultiSparseFeatures *features,
        Combo &combo) const
{
    for (size_t i = 0; i < inputs.size(); i++) {
        FeatureInput *input = inputs[i];
#define CASE(vt)                                                        \
        case vt:                                                        \
        {                                                               \
            typedef InputType2Type<vt>::Type T;                         \
            typedef FeatureInputTyped<T, DenseStorage<T>> FeatureInputTyped; \
            FeatureInputTyped *typedInput = static_cast<FeatureInputTyped*>(input); \
            if (!typedInput) {                                          \
                return;                                                 \
            }                                                           \
            size_t r = typedInput->row() == 1 ? 0 : id;                 \
            if(!combo.collect(typedInput, r, 0)) {                      \
                return;                                                 \
            }                                                           \
        }                                                               \
        break

        switch(input->dataType()) {
            INPUT_DATA_TYPE_MACRO_HELPER(CASE);
        default:
            break;
        }
#undef CASE
        if (i == inputs.size() - 1) {
            auto buffer = combo.getWholeBuf();
            features->addFeatureKey(buffer.data(), buffer.size());
        } else {
            combo.addSeparator();
        }
    }
}

template<typename Combo>
void ComboFeatureFunction::genFeatureNormal(
        const std::vector<FeatureInput*> &inputs, size_t docId, size_t featureId,
        MultiSparseFeatures *features,
        Combo &combo) const
{
    if (featureId >= inputs.size()) {
        return;
    }
    FeatureInput *input = inputs[featureId];
#define APPEND_ONE_FEATURE_TYPED(t)                                     \
    case t:                                                             \
    {                                                                   \
        typedef InputType2Type<t>::Type T;                              \
        if (input->storageType() == IST_DENSE) {                        \
            appendOneFeature<T, DenseStorage<T>, Combo>(                \
                    inputs, docId, featureId, features, combo);         \
        } else if (input->storageType() == IST_SPARSE_MULTI_VALUE) {    \
            appendOneFeature<T, MultiValueStorage<T>, Combo>(           \
                    inputs, docId, featureId, features, combo);         \
        } else {                                                        \
            appendOneFeature<T, ValueOffsetStorage<T>, Combo>(          \
                    inputs, docId, featureId, features, combo);         \
        }                                                               \
    }                                                                   \
    break

    switch(input->dataType()) {
        INPUT_DATA_TYPE_MACRO_HELPER(APPEND_ONE_FEATURE_TYPED);
    default:
        break;
    }
#undef APPEND_ONE_FEATURE_TYPED
}

template <typename T, typename StorageType, typename Combo>
void ComboFeatureFunction::appendOneFeature(
        const vector<FeatureInput*> &inputs,
        size_t docId,
        size_t featureId,
        MultiSparseFeatures *features,
        Combo &combo) const
{
    FeatureInput *input = inputs[featureId];
    typedef FeatureInputTyped<T, StorageType> FeatureInputTyped;
    FeatureInputTyped *typedInput = static_cast<FeatureInputTyped*>(input);
    const size_t beginPos = combo.getBufferLength();
    size_t r = typedInput->row() == 1 ? 0 : docId;
    size_t left, right;
    if (_pruneRight[featureId]) {
        left = 0;
        right = min(size_t(_pruneLimit[featureId]), typedInput->col(r));
    } else {
        left = typedInput->col(r) - min(_pruneLimit[featureId], int(typedInput->col(r)));
        right = typedInput->col(r);
    }
    for (size_t c = left; c < right; c++) {
        if (!combo.collect(typedInput, r, c)) {
            continue;
        }
        if (featureId + 1 == inputs.size()) {
            auto buffer = combo.getWholeBuf();
            features->addFeatureKey(buffer.data(), buffer.size()); //
        } else {
            combo.addSeparator();
            genFeatureNormal<Combo>(inputs, docId, featureId+1, features, combo);
        }
        combo.backTrace(beginPos);
    }
}

}
