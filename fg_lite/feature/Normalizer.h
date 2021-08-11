#ifndef ISEARCH_FG_LITE_NORMALIZER_H
#define ISEARCH_FG_LITE_NORMALIZER_H

#include <cmath>
#include "autil/Log.h"
#include <algorithm>
#include "fg_lite/feature/Feature.h"

namespace fg_lite {

class Normalizer
{
public:
    Normalizer()
        : _type(UNKNOWN)
        , _param1(0)
        , _param2(1)
    {}
    ~Normalizer() {}

public:
    enum NormalizerType {
        RAW,
        MINMAX,
        ZSCORE,
        LOG10,
        BOUND_REGULAR,
        UNKNOWN,
    };
public:
    bool parse(const std::string &description);

    float normalize(float x) const {
        switch (_type) {
        case MINMAX:
            return minmax(x, _param1, _param2);
        case ZSCORE:
            return zscore(x, _param1, _param2);
        case LOG10:
            return log10(x, _param1, _param2);
        case BOUND_REGULAR:
            return bound_regular(x,
                    _param1,
                    _param2,
                    _param3,
                    _param4,
                    _param5,
                    _switch1);
        default:
            return x;
        }
    }
    
private:
    static float minmax(float x, float min, float max) {
        return (x - min) / (max - min);
    }

    static float zscore(float x, float mean, float sd) {
        return (x - mean) / sd;
    }

    static float log10(float x, float threshold, float defaultValue) {
        return x > threshold ? ::log10(x) : defaultValue;
    }

    static float min(float a, float b) { return a > b ? b : a; }
    static float max(float a, float b) { return a < b ? b : a; }
    static float bound_regular(float x, float mean, float sd,
                               float y_add, float y_min, float y_max, bool log) {
        float minimum = Normalizer::min(Normalizer::max(x, y_max) + y_add, y_min);
        float input = log ? ::log(minimum) : minimum;
        return (input - mean) / sd;
    }

private:
    typedef std::vector<std::pair<std::string, std::string> > KVPairVec;
    template<typename T>
    bool setParam(const KVPairVec &kvPairVec, 
                  const std::string &key, T &param);
    bool parseParam(const KVPairVec  &kvPairVec);
    bool parseMinMax(const KVPairVec &kvPairVec);
    bool parseZscore(const KVPairVec &kvPairVec);
    bool parseLog10(const KVPairVec &kvPairVec);
    bool parseBoundRegular(const KVPairVec &kvPairVec);

    static const std::string METHOD;
    static const std::string MIN;
    static const std::string MAX;
    static const std::string MEAN;
    static const std::string STANDARD_DEVIATION;
    static const std::string THRESHOLD;
    static const std::string Y_ADD;
    static const std::string Y_MIN;
    static const std::string Y_MAX;
    static const std::string LOG;
    static const std::string DEFAULT_VALUE;

public:
    NormalizerType _type;
    float _param1;
    float _param2;
    float _param3{0.0f};
    float _param4{0.0f};
    float _param5{0.0f};
    bool _switch1{false};

private:
    AUTIL_LOG_DECLARE();
};

inline int64_t bucketize(float value, const std::vector<float> &boundaries) {
    return std::upper_bound(
            boundaries.begin(),
            boundaries.end(),
            value) - boundaries.begin();
}

inline void addFeatureMayBucketize(SingleDenseFeatures *features, const std::vector<float> &boundaries, float value) {
    features->addFeatureValue(value);
}

inline void addFeatureMayBucketize(SingleIntegerFeatures *features, const std::vector<float> &boundaries, float value) {
    features->addFeatureValue(bucketize(value, boundaries));
}

}

#endif //ISEARCH_FG_LITE_NORMALIZER_H
