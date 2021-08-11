#include "fg_lite/feature/LookupFeatureFunction.h"
#include "autil/StringUtil.h"

using namespace std;
using namespace autil;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, LookupFeatureFunction);

const char KEY_VALUE_SEPARATOR = ':';
const char VALUE_MULTIVALUE_SEPARATOR = ',';

LookupFeatureFunction::LookupFeatureFunction(
        const string &name,
        const string &prefix,
        bool needDiscrete,
        bool needKey,
        const Normalizer &normalizer,
        const std::string &combiner,
        uint32_t dimension,
        bool needWeighting,
        bool isOptimized,
        const vector<float> &boundaries,
        const string &defaultLookupResult,
        bool hasDefault)
    : FeatureFunction(name, prefix)
    , _normalizer(normalizer)
    , _combinerType(combinerConvert(combiner))
    , _dimension(dimension)
    , _needDiscrete(needDiscrete)
    , _needKey(needKey)
    , _needWeighting(needWeighting)
    , _isOptimized(isOptimized)
    , _hasDefault(hasDefault)
    , _defaultLookupResult(defaultLookupResult)
    , _boundaries(boundaries)
{
}

struct WriterArgs {
    WriterArgs(bool needKey_, uint32_t dimension_, Normalizer normalizer_, const vector<float> &boundaries_)
        : needKey(needKey_)
        , dimension(dimension_)
        , normalizer(normalizer_)
        , boundaries(boundaries_)
    {
    }
    bool needKey;
    uint32_t dimension;
    Normalizer normalizer;
    vector<float> boundaries;
};

class SparseFeatureWriter {
public:
    typedef MultiSparseFeatures FeaturesType;
public:
    SparseFeatureWriter(FeaturesType *f, const WriterArgs &args)
        : features(f)
        , _needKey(args.needKey)
    {
        features->beginDocument();
    }
public:
    static FeaturesType *createFeatures(const string &name, uint32_t reserveSize) {
        return new FeaturesType(reserveSize);
    }

    void addFeature(const ConstString &key, const ConstString &value,
                    FeatureFormatter::FeatureBuffer &buffer, uint32_t idx)
    {
        size_t beginPos = buffer.size();
        if (_needKey) {
            buffer.insert(buffer.end(), key.begin(), key.end());
            buffer.push_back(FEATURE_SEPARATOR);
        }
        buffer.insert(buffer.end(), value.begin(), value.end());
        features->addFeatureKey(buffer.data(), buffer.size());
        buffer.assign(buffer.begin(), buffer.begin() + beginPos);
    }
public:
    FeaturesType *features;
    bool _needKey;
};

class SparseWeightingFeatureWriter {
public:
    typedef MultiSparseWeightingFeatures FeaturesType;
public:
    SparseWeightingFeatureWriter(FeaturesType *f, const WriterArgs &args)
        : features(f)
        , _needKey(args.needKey)
    {
        features->beginDocument();
    }
public:
    static FeaturesType *createFeatures(const string &name, uint32_t docCount) {
        return new FeaturesType(docCount);
    }
    void addFeature(const ConstString &key, const ConstString &value,
                    FeatureFormatter::FeatureBuffer &buffer, uint32_t idx)
    {
        size_t beginPos = buffer.size();
        if (_needKey) {
            buffer.insert(buffer.end(), key.begin(), key.end());
        }
        features->addFeatureKey(buffer.data(), buffer.size());
        double v = StringUtil::fromString<double>(string(value.data(), value.size()));
        features->addFeatureValue(v);

        buffer.assign(buffer.begin(), buffer.begin() + beginPos);
    }
public:
    FeaturesType *features;
    bool _needKey;
};

template<typename CombinerT, typename FeaturesT>
class DenseFeatureWriter {
public:
    typedef FeaturesT FeaturesType;
    DenseFeatureWriter(FeaturesType *f, const WriterArgs &args)
        : features(f)
        , normalizer(args.normalizer)
        , boundaries(args.boundaries)
    {
    }
    ~DenseFeatureWriter() {
        addFeatureMayBucketize(features, boundaries, normalizer.normalize(combiner.get()));
    }
public:
    static FeaturesType *createFeatures(const string &name, uint32_t reserveSize) {
        return new FeaturesType(name, reserveSize);
    }
    void addFeature(const ConstString &key, const ConstString &value,
                    FeatureFormatter::FeatureBuffer &buffer, uint32_t idx)
    {
        float v = StringUtil::fromString<float>(string(value.data(), value.size()));
        combiner.collect(v);
    }
public:
    FeaturesType *features;
    CombinerT combiner;
    const Normalizer &normalizer;
    const vector<float> &boundaries;
};

template<typename CombinerT>
class MultiDenseFeatureWriter {
public:
    typedef MultiDenseFeatures FeaturesType;
public:
    MultiDenseFeatureWriter(FeaturesType *f, const WriterArgs &args)
        : features(f)
        , normalizer(args.normalizer)
        , _combiners(vector<CombinerT>(args.dimension))
        , _dimension(args.dimension)
    {
        features->beginDocument();
        features->_featureValues.resize(features->_featureValues.size() + _dimension);
        valueBuffer = features->_featureValues.data() + features->_featureValues.size() - _dimension;
    }
    ~MultiDenseFeatureWriter() {
        for (auto i = 0; i < _dimension; i++) {
            valueBuffer[i] = _combiners[i].get();
        }
    }
public:
    static FeaturesType *createFeatures(const string &name, uint32_t reserveSize) {
        return new FeaturesType(name, reserveSize);
    }
    void addFeature(const ConstString &key, const ConstString &value, FeatureFormatter::FeatureBuffer &buffer, uint32_t idx) {
        vector<string> values;
        StringUtil::split(values, string(value.data(), value.size()), VALUE_MULTIVALUE_SEPARATOR, true);
        for (size_t i = 0; i < values.size(); i++) {
            if (i >= _dimension) {
                return;
            }
            float v = StringUtil::fromString<float>(values[i]);
            _combiners[i].collect(v);
        }
    }
public:
    float *valueBuffer;
    FeaturesType *features;
    const Normalizer &normalizer;
    vector<CombinerT> _combiners;
    uint32_t _dimension;
};

bool checkType(FeatureInput *input) {
    // multistring or just cstring
    bool allowed = (input->dataType() == IT_CSTRING
                    && input->storageType() == IST_DENSE)
                   ||
                   (input->dataType() == IT_STRING
                    && input->storageType() == IST_SPARSE_MULTI_VALUE);
    return allowed;
}

Features *LookupFeatureFunction::genFeatures(
        const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
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

    if (_isOptimized) {
        FeatureInput *valueInput = inputs[0];
        if (!checkType(valueInput)) {
            AUTIL_LOG(WARN, "feature[%s]: value type invalid, storageType: %d, dataType: %d",
                      getFeatureName().c_str(), (int)valueInput->storageType(),
                      (int)valueInput->dataType());
            return nullptr;
        }

        if (_needDiscrete) {
            if (_needWeighting) {
                AUTIL_LOG(ERROR, "value is optimized cannot be weighting");
                return nullptr;
            }
            return genOptimizedFeatureTemplate<SparseFeatureWriter>(valueInput, context);
        } else if (1 == _dimension) {
#define TEMPLATE_1(type)                                                \
            return genOptimizedFeatureTemplate<DenseFeatureWriter<Combiner<type>, SingleDenseFeatures>>(valueInput, context);
#define TEMPLATE_2(type)                                                \
            return genOptimizedFeatureTemplate<DenseFeatureWriter<Combiner<type>, SingleIntegerFeatures>>(valueInput, context);
            if (_boundaries.empty()) {
                COMBINER_ENUM(_combinerType, TEMPLATE_1);
            } else {
                COMBINER_ENUM(_combinerType, TEMPLATE_2);
            }
#undef TEMPLATE_1
#undef TEMPLATE_2
        } else {
#define TEMPLATE_3(type)                                                \
            return genOptimizedFeatureTemplate<MultiDenseFeatureWriter<Combiner<type>>>(valueInput, context);

            COMBINER_ENUM(_combinerType, TEMPLATE_3);
#undef TEMPLATE_3
        }
    }

    FeatureInput *mapInput = inputs[0];
    FeatureInput *keyInput = inputs[1];

    if (!checkType(mapInput)) {
        AUTIL_LOG(DEBUG, "feature[%s]: map type invalid, storageType: %d, dataType: %d",
                  getFeatureName().c_str(), (int)mapInput->storageType(),
                  (int)mapInput->dataType());
        return nullptr;
    }
    if (_needDiscrete) {
        if (_needWeighting) {
            return genFeatureTemplate<SparseWeightingFeatureWriter>(
                    mapInput, keyInput, context);
        }
        return genFeatureTemplate<SparseFeatureWriter>(mapInput, keyInput, context);
    } else if (1 == _dimension) {
#define TEMPLATE_1(type)                                                \
        return genFeatureTemplate<DenseFeatureWriter<Combiner<type>, SingleDenseFeatures>>(mapInput, keyInput, context);
#define TEMPLATE_2(type)                                                \
        return genFeatureTemplate<DenseFeatureWriter<Combiner<type>, SingleIntegerFeatures>>(mapInput, keyInput, context);
        if (_boundaries.empty()) {
            COMBINER_ENUM(_combinerType, TEMPLATE_1);
        } else {
            COMBINER_ENUM(_combinerType, TEMPLATE_2);
        }
#undef TEMPLATE_1
#undef TEMPLATE_2
    } else {
#define TEMPLATE_3(type)   \
            return genFeatureTemplate<MultiDenseFeatureWriter<Combiner<type>>>(mapInput, keyInput, context);

        COMBINER_ENUM(_combinerType, TEMPLATE_3);
#undef TEMPLATE_3
    }
}

template <class FeatureWriter>
void insertSparseFeature(FeatureWriter &writer,
                         const LookupFeatureFunction::ConstStringVec &keys,
                         const LookupFeatureFunction::ConstStringMap &mapDict,
                         FeatureFormatter::FeatureBuffer &buffer,
                         typename FeatureWriter::FeaturesType *features,
                         bool hasDefault,
                         ConstString defaultLookupResult)
{
    for (size_t i = 0; i < keys.size(); i++) {
        const auto &key = keys[i];
        auto it = mapDict.find(key);
        if (it == mapDict.end()) {
            if (!hasDefault) {
                continue;
            } else {
                // 可以考虑用 SingleSparseFeature 优化掉 offset
                writer.addFeature(key, ConstString(defaultLookupResult), buffer, i);
            }
        } else {
            writer.addFeature(key, it->second, buffer, i);
        }
    }
}

template <class FeatureWriter>
void insertOptimizedSparseFeature(FeatureWriter &writer,
                                  const LookupFeatureFunction::ConstStringVec &values,
                                  FeatureFormatter::FeatureBuffer &buffer,
                                  typename FeatureWriter::FeaturesType *features)
{
    for (size_t i = 0; i < values.size(); i++) {
        ConstString empty("");
        writer.addFeature(empty, values[i], buffer, i);
    }
}

template <class FeatureWriter>
Features *LookupFeatureFunction::genFeatureTemplate(
        FeatureInput *mapInput,
        FeatureInput *keyInput,
        FeatureFunctionContext *context) const
{
    size_t count = std::max(mapInput->row(), keyInput->row());
    auto features = FeatureWriter::createFeatures(getFeatureName(), count);
    auto pool = features->getPool();
    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(pool);

    bool keyFlag = keyInput->row() == 1;
    bool mapFlag = mapInput->row() == 1;
    bool keyNeedCopy = !checkType(keyInput);
    FeatureFormatter::FeatureBuffer keyBuffer{cp_alloc(pool)};
    ConstStringVec keys{vec_alloc(pool)};
    keys.reserve(8);
    ConstStringMap mapDict{map_alloc(pool)};
    mapDict.reserve(32);

    WriterArgs args{_needKey, _dimension, _normalizer, _boundaries};
    for (size_t i = 0 ; i < count; i++) {
        if (!keyFlag || i == 0) {
            keys.clear();
            if (!keyNeedCopy) {
                collectValues(keyInput, i, [&keys](const ConstString &v){
                            keys.push_back(ConstString(v));
                        });
            } else {
                // need copy
                keyBuffer.clear();
                collectKeys(keyInput, i, keyBuffer, keys);
            }
        }
        if (!mapFlag || i == 0) {
            mapDict.clear();
            collectValues(mapInput, i, [&mapDict](const ConstString &v){
                        size_t pos = v.find(KEY_VALUE_SEPARATOR);
                        if (pos == string::npos) {
                            return;
                        }
                        ConstString key(v.data(), pos);
                        ConstString value(v.data() + pos + 1, v.size() - pos - 1);
                        mapDict[key] = value;
                    });
        }
        FeatureWriter writer(features, args);
        insertSparseFeature<FeatureWriter>(writer, keys, mapDict, buffer, features,
                _hasDefault, ConstString(_defaultLookupResult));
    }

    return features;
}

template <class FeatureWriter>
Features *LookupFeatureFunction::genOptimizedFeatureTemplate(
        FeatureInput *valueInput,
        FeatureFunctionContext *context) const
{
    size_t count = valueInput->row();
    auto features = FeatureWriter::createFeatures(getFeatureName(), count);
    auto pool = features->getPool();
    FeatureFormatter::FeatureBuffer buffer = getFeaturePrefix(pool);

    FeatureFormatter::FeatureBuffer keyBuffer{cp_alloc(pool)};
    ConstStringVec values{vec_alloc(pool)};
    values.reserve(8);

    WriterArgs args{_needKey, _dimension, _normalizer, _boundaries};
    for (size_t i = 0 ; i < count; i++) {
        values.clear();
        keyBuffer.clear();
        collectKeys(valueInput, i, keyBuffer, values);
        FeatureWriter writer(features, args);
        insertOptimizedSparseFeature<FeatureWriter>(writer, values, buffer, features);
    }

    return features;
}

void LookupFeatureFunction::collectKeys(FeatureInput *key, size_t rowId,
                                        FeatureFormatter::FeatureBuffer &keyBuffer,
                                        ConstStringVec &keys) const
{
    assert(rowId < key->row());
    pool_vector<size_t> posVec(mem_pool::pool_allocator<size_t>(keyBuffer.get_allocator()._pool));
    size_t numKeys = key->col(rowId);
    posVec.reserve(numKeys);
    keys.reserve(numKeys);
    for (size_t i = 0; i < numKeys; i++) {
        posVec.push_back(keyBuffer.size());
        key->toString(rowId, i, keyBuffer);
    }
    for (size_t i = 0; i < numKeys; i++) {
        const char *data = keyBuffer.data() + posVec[i];
        size_t len = i + 1 == numKeys?
                     (keyBuffer.size() - posVec[i])
                     : (posVec[i+1] - posVec[i]);
        if (len != 0) {
            keys.push_back(ConstString(data, len));
        }
    }
}

template<typename CollectFun>
bool LookupFeatureFunction::collectValues(
        FeatureInput *input,
        size_t rowId,
        CollectFun func) const
{
    assert(rowId < input->row());
    if (input->dataType() == IT_CSTRING && input->storageType() == IST_DENSE) {
        typedef FeatureInputTyped<string, DenseStorage<string>> FeatureInputTyped;
        FeatureInputTyped *typedInput = static_cast<FeatureInputTyped*>(input);
        for (size_t c = 0; c < typedInput->col(rowId); c++) {
            const string &value = typedInput->get(rowId, c);
            ConstString cs(value.data(), value.size());
            func(cs);
        }
    } else if (input->dataType() == IT_STRING
               && input->storageType() == IST_SPARSE_MULTI_VALUE)
    {
        typedef FeatureInputTyped<MultiChar, MultiValueStorage<MultiChar>> FeatureInputTyped;
        FeatureInputTyped *typedInput = static_cast<FeatureInputTyped*>(input);
        for (size_t c = 0; c < typedInput->col(rowId); c++) {
            MultiChar value = typedInput->get(rowId, c);
            ConstString cs(value.data(), value.size());
            func(cs);
        }
    } else {
        AUTIL_LOG(WARN, "type unexpected");
        return false;
    }
    return true;
}

}
