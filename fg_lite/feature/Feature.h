#ifndef ISEARCH_FG_LITE_FEATURE_H
#define ISEARCH_FG_LITE_FEATURE_H

#include "autil/StringUtil.h"
#include "autil/ConstString.h"
#include "autil/mem_pool/Pool.h"
#include "autil/mem_pool/pool_allocator.h"

namespace fg_lite {

using cp_alloc = autil::mem_pool::pool_allocator<char>;

template<typename T>
using pool_vector = std::vector<T, autil::mem_pool::pool_allocator<T>>;

enum FeatureValueType {
    FVT_SINGLE_SPARSE,
    FVT_SINGLE_DENSE,
    FVT_MULTI_SPARSE,
    FVT_MULTI_DENSE,
    FVT_TENSOR_SPARSE,
    FVT_WEIGHTING_SPARSE,
    FVT_SINGLE_SPARSE_INT,
    FVT_MULTI_SPARSE_INT
};

class Features {
public:
    Features(FeatureValueType type)
        : _type(type)
    {
    }
    virtual ~Features() {}
public:
    FeatureValueType getFeatureValueType() const { return _type; }
    virtual size_t count() const = 0;
protected:
    void setFeatureValueType(FeatureValueType type)
    { _type = type; }
private:
    FeatureValueType _type;
};

/*
 * only vector<HashKey>
 */
class SingleSparseFeatures : public Features {
public:
    SingleSparseFeatures(size_t reserveSize)
        : Features(FVT_SINGLE_SPARSE)
        , _pool(1024)
        , _featureNames(&_pool)
    {
        _featureNames.reserve(reserveSize);
    }
    ~SingleSparseFeatures() = default;
public:
    size_t count() const override {
        return _featureNames.size();
    }
    void addFeatureKey(const char *key, size_t len) {
        _featureNames.push_back(autil::ConstString(key, len, &_pool));
    }
    // convert bufferVec into buffer

    autil::mem_pool::UnsafePool _pool;
public:
    autil::mem_pool::Pool *getPool() {
        return &_pool;
    }
    const pool_vector<autil::ConstString> &getFeatures() const {
        return _featureNames;
    }
    pool_vector<autil::ConstString> _featureNames;
};

/*
 * vector<offset> and vector<HashKey>
 */
class MultiSparseFeatures : public SingleSparseFeatures {
public:
    MultiSparseFeatures(size_t reserveSize)
        : SingleSparseFeatures(reserveSize)
        , _offsets(&_pool)
    {
        setFeatureValueType(FVT_MULTI_SPARSE);
        _offsets.reserve(reserveSize);
    }
    ~MultiSparseFeatures() = default;
public:
    size_t count() const override {
        return _offsets.size();
    }
    void beginDocument() {
        _offsets.push_back(_featureNames.size());
    }
public:
    pool_vector<size_t> _offsets;
};


/*
 * vector<offset> and vector<value> for both feature_name and feature_value
 */
class MultiSparseWeightingFeatures : public MultiSparseFeatures {
public:
    MultiSparseWeightingFeatures(size_t reserveSize)
        : MultiSparseFeatures(reserveSize)
        , _featureValues(&_pool)
    {
        setFeatureValueType(FVT_WEIGHTING_SPARSE);
        _featureValues.reserve(reserveSize);
    }
    ~MultiSparseWeightingFeatures()  = default;

public:
    void addFeatureValue(double value) {
        _featureValues.push_back(value);
    }
public:
    pool_vector<double> _featureValues;
};

/*
 * only vector<float>
 */
class SingleDenseFeatures : public Features {
public:
    SingleDenseFeatures(const std::string &featureName, size_t reserveSize)
        : Features(FVT_SINGLE_DENSE)
        , _pool(1024)
        , _featureValues(&_pool)
        , _featureName(featureName)
    {
        _featureValues.reserve(reserveSize);
    }
    ~SingleDenseFeatures() = default;
public:
    void addFeatureValue(float featureValue) {
        _featureValues.push_back(featureValue);
    }
    float getFeatureValue(uint32_t idx) const {
        assert(idx < _featureValues.size());
        return _featureValues[idx];
    }
    size_t valueSize() const {
        return _featureValues.size();
    }
    size_t count() const override {
        return _featureValues.size();
    }
    void beginDocument() {};
protected:
    autil::mem_pool::UnsafePool _pool;
public:
    autil::mem_pool::Pool *getPool() {
        return &_pool;
    }
    pool_vector<float> _featureValues;
    std::string _featureName;
};

/*
 * vector<offset> and vector<float>
 */
class MultiDenseFeatures : public SingleDenseFeatures {
public:
    MultiDenseFeatures(const std::string &featureName, size_t reserveSize)
        : SingleDenseFeatures(featureName, reserveSize)
        , _offsets(&_pool)
    {
        setFeatureValueType(FVT_MULTI_DENSE);
        _offsets.reserve(reserveSize);
    }
    ~MultiDenseFeatures()  = default;
public:
    void beginDocument() {
        _offsets.push_back(_featureValues.size());
    }
    size_t count() const override {
        return _offsets.size();
    }
public:
    pool_vector<size_t> _offsets;
};

template <typename ValueType, typename Work>
void forEachSingleFeature(const pool_vector<ValueType> &values, Work &work) {
    for (size_t i = 0; i < values.size(); ++i) {
        work.handleOne(i, values[i]);
    }
}

template <typename ValueType, typename Work>
void forEachMultiFeature(const pool_vector<ValueType> &values,
                         const pool_vector<size_t> &offsets,
                         Work &work)
{
    for (size_t i = 0; i < values.size(); ++i) {
        size_t begin = offsets[i];
        size_t next = i + 1;
        size_t end = (next != offsets.size()) ? offsets[next] : values.size();
        const ValueType *addr = values.data();
        work.handleMulti(i, addr + begin, addr + end);
    }
}

/*
 * only vector<int64>
 */
class SingleIntegerFeatures : public Features {
public:
    SingleIntegerFeatures(const std::string &featureName, size_t reserveSize)
        : Features(FVT_SINGLE_SPARSE_INT)
        , _pool(1024)
        , _featureValues(&_pool)
        , _featureName(featureName)
    {
        _featureValues.reserve(reserveSize);
    }
    ~SingleIntegerFeatures()  = default;
public:
    size_t count() const override {
        return _featureValues.size();
    }
    void addFeatureValue(int64_t featureValue) {
        _featureValues.push_back(featureValue);
    }
protected:
    autil::mem_pool::UnsafePool _pool;
public:
    autil::mem_pool::Pool *getPool() {
        return &_pool;
    }
    const pool_vector<int64_t> &getFeatures() const {
        return _featureValues;
    }
    pool_vector<int64_t> _featureValues;
    std::string _featureName;
};

/*
 * vector<offset> and vector<int64>
 */
class MultiIntegerFeatures : public SingleIntegerFeatures {
public:
    MultiIntegerFeatures(const std::string &featureName, size_t reserveSize)
        : SingleIntegerFeatures(featureName, reserveSize)
        , _offsets(&_pool)
    {
        setFeatureValueType(FVT_MULTI_SPARSE_INT);
        _offsets.reserve(reserveSize);
    }
    ~MultiIntegerFeatures()  = default;
public:
    size_t count() const override {
        return _offsets.size();
    }
    void beginDocument() {
        _offsets.push_back(_featureValues.size());
    }
public:
    pool_vector<size_t> _offsets;
};

struct MultiFeatureType {};
struct SingleFeatureType {};

template<typename T, bool =
         std::is_same<T, MultiIntegerFeatures>::value ||
         std::is_same<T, MultiSparseFeatures>::value ||
         std::is_same<T, MultiDenseFeatures>::value>
    struct MultiFeatureTrait {
        typedef SingleFeatureType type;
    };

template<typename T>
struct MultiFeatureTrait<T, true> {
    typedef MultiFeatureType type;
};

}

#endif //ISEARCH_FG_LITE_FEATURE_H
