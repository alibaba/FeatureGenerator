#ifndef ISEARCH_FG_LITE_COMBINER_H
#define ISEARCH_FG_LITE_COMBINER_H

#include "autil/Log.h"
#include <limits>
#include <string>
#include <algorithm>

namespace fg_lite {

enum class CombinerType {
    MEAN,
    SUM,
    MIN,
    MAX,
    GAP_MIN,
    GAP_MAX,
    COUNT,
    NONE
};

inline CombinerType combinerConvert(const std::string &combiner) {
    if (combiner == "min") {
        return CombinerType::MIN;
    } else if (combiner == "max") {
        return CombinerType::MAX;
    } else if (combiner == "mean" || combiner == "avg") {
        return CombinerType::MEAN;
    } else if (combiner == "gap_min") {
        return CombinerType::GAP_MIN;
    } else if (combiner == "gap_max") {
        return CombinerType::GAP_MAX;
    } else if (combiner == "count") {
        return CombinerType::COUNT;
    } else if (combiner == "none") {
      return CombinerType::NONE;
    } else {
        return CombinerType::SUM;
    }
}


template<CombinerType type>
class Combiner
{
public:
    Combiner()
        : _value(0)
        , _count(0)
    {
    }
    ~Combiner()  = default;
private:
    Combiner(const Combiner &);
    Combiner& operator=(const Combiner &);
public:
    void collect(float value) {
        // if value count == 0, default result of max/min combiner == 0
        _count++;
        switch (type) {
        case CombinerType::MIN:
            _value = 1 == _count ? value : std::min(value, _value);
            break;
        case CombinerType::MAX:
            _value = 1 == _count ? value : std::max(value, _value);
            break;
        case CombinerType::SUM:
        case CombinerType::MEAN:
            _value += value;
            break;
        case CombinerType::COUNT:
            break;
        // GAP_MIN GAP_MAX ...
        }
    }
    float get() const {
        if (CombinerType::COUNT == type) {
            return _count * 1.0f;
        }
        if (CombinerType::MEAN == type && _count > 0) {
            return _value / _count;
        }
        return _value;
    }
public:
    float _value;
    uint32_t _count;
private:
    AUTIL_LOG_DECLARE();
};

template <CombinerType type>
class MultiDimensionCombiner {
public:
    MultiDimensionCombiner(float *buffer, const uint32_t dimension)
        : _buffer(buffer)
        , _dimension(dimension)
        , _counter(0)
        , _finalized(false)
    {
    }
public:
    void collect(const float *inputs) {
        ++_counter;
        switch (type) {
        case CombinerType::SUM:
        case CombinerType::MEAN:
            add(inputs);
            break;
        case CombinerType::MIN:
            min(inputs);
            break;
        case CombinerType::MAX:
            max(inputs);
            break;
        default:
            break;
        }
    }

    float *get() {
        if (_finalized) {
            return _buffer;
        }
        if (CombinerType::MEAN == type && _counter  > 0) {
            std::transform(_buffer, _buffer + _dimension, _buffer,
                           [this](float v) { return v / _counter; });
        }
        _finalized = true;
        return _buffer;
    }
public:
    void add(const float *inputs) {
        std::transform(_buffer, _buffer + _dimension, inputs, _buffer, std::plus<float>());
    }

    void min(const float *inputs) {
        if (_counter == 1) {
            std::copy(inputs, inputs + _dimension, _buffer);
        } else {
            auto fn = [](float x, float y) { return std::min(x, y); };
            std::transform(_buffer, _buffer + _dimension, inputs, _buffer, std::move(fn));
        }
    }
    void max(const float *inputs) {
        if (_counter == 1) {
            std::copy(inputs, inputs + _dimension, _buffer);
        } else {
            auto fn = [](float x, float y) { return std::max(x, y); };
            std::transform(_buffer, _buffer + _dimension, inputs, _buffer, std::move(fn));
        }
    }
private:
    float *_buffer;
    const uint32_t _dimension;
    uint32_t _counter;
    bool _finalized;
};

}


#define COMBINER_ENUM_WITH_INDEX(type, macro, i)   \
    switch (type) {                                \
    case CombinerType::MEAN:                       \
        macro(CombinerType::MEAN, i);              \
    case CombinerType::MIN:                        \
        macro(CombinerType::MIN, i);               \
    case CombinerType::MAX:                        \
        macro(CombinerType::MAX, i);               \
    case CombinerType::SUM:                        \
    default:                                       \
        macro(CombinerType::SUM, i);               \
    }

#define COMBINER_ENUM(type, macro)              \
    switch (type) {                             \
    case CombinerType::MEAN:                    \
        macro(CombinerType::MEAN);              \
    case CombinerType::MIN:                     \
        macro(CombinerType::MIN);               \
    case CombinerType::MAX:                     \
        macro(CombinerType::MAX);               \
    case CombinerType::SUM:                     \
    default:                                    \
        macro(CombinerType::SUM);               \
        }



#endif //ISEARCH_FG_LITE_COMBINER_H
