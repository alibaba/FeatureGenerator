#pragma once

#include "fg_lite/feature/Normalizer.h"
#include "fg_lite/feature/Combiner.h"
#include "fg_lite/feature/Feature.h"

namespace fg_lite {


template <CombinerType CT>
class MultiDimensionCollector {
public:
    MultiDimensionCollector(const std::vector<uint64_t> *keys,
                            float* buffer,
                            const uint32_t dimension)
        : _keys(keys)
        , _combiner(buffer, dimension)
        , _iter(0)
    {}
    ~MultiDimensionCollector() {
        _combiner.get();
    }
private:
    MultiDimensionCollector(const MultiDimensionCollector &);
    MultiDimensionCollector& operator=(const MultiDimensionCollector &);
public:
    bool end() const {
        return _iter >= _keys->size();
    }
    uint64_t getKey() {
        return (*_keys)[_iter++];
    }

    void collect(float *buffer) {
        _combiner.collect(buffer);
    }
private:
    const std::vector<uint64_t>* _keys;
    MultiDimensionCombiner<CT> _combiner;
    uint32_t _iter;
};

template<typename CombinerT, typename FeatureT>
class SingleCollector {
public:
    SingleCollector(const std::vector<uint64_t> *keys,
                    const Normalizer *normalizer,
                    const std::vector<float> *boundaries,
                    FeatureT *features)
        : _keys(keys)
        , _iter(0)
        , _normalizer(normalizer)
        , _boundaries(boundaries)
        , _features(features)
    {
    }
    ~SingleCollector() {
        addFeatureMayBucketize(_features, *_boundaries, _normalizer->normalize(_combiner.get()));
    }
    uint64_t getKey() {
        return (*_keys)[_iter++];
    }
    bool end() const {
        return _iter >= _keys->size();
    }

    template<typename T>
    void collect(T *buffer) {
        float value = *buffer;
        _combiner.collect(value);
    }

private:
    const std::vector<uint64_t> *_keys;
    uint32_t _iter;
    CombinerT _combiner;
    const Normalizer *_normalizer;
    const std::vector<float> *_boundaries;
    FeatureT *_features;
};

template<typename CombinerT, typename FeatureT>
class MultiCollectorV3 {
public:
    MultiCollectorV3(const std::vector<uint64_t> *keys,
                     uint32_t dimension,
                     const Normalizer *normalizer,
                     const std::vector<float> *boundaries,
                     FeatureT *features)
        : _keys(keys)
        , _iter(0)
        , _combiners(std::vector<CombinerT>(dimension))
        , _normalizer(normalizer)
        , _boundaries(boundaries)
        , _features(features)
    {
    }
    // supoort multi Collector
    ~MultiCollectorV3() {
        for (auto const &combiner: _combiners) {
            addFeatureMayBucketize(_features, *_boundaries, _normalizer->normalize(combiner.get()));
        }
    }

    void beginCollect() {
        _iter = 0;
    }

    uint64_t getKey() {
        return (*_keys)[_iter++];
    }

    bool end() const {
        return _iter >= _keys->size();
    }
    void collect(float value, uint32_t fieldIndex) {
        _combiners[fieldIndex].collect(value);
    }

    size_t size() {
        return _keys->size();
    }

    const std::vector<uint64_t> &getKeys() {
        return *_keys;
    }

private:
    const std::vector<uint64_t> *_keys;
    uint32_t _iter;
    std::vector<CombinerT> _combiners;
    const Normalizer *_normalizer;
    const std::vector<float> *_boundaries;
    FeatureT *_features;
};

}
