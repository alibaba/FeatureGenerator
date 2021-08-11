#include "fg_lite/feature/Normalizer.h"
#include "autil/StringUtil.h"

using namespace std;
using namespace autil;

namespace fg_lite {
AUTIL_LOG_SETUP(fg_lite, Normalizer);

const string Normalizer::METHOD = "method";
const string Normalizer::MIN = "min";
const string Normalizer::MAX = "max";
const string Normalizer::MEAN = "mean";
const string Normalizer::STANDARD_DEVIATION = "standard_deviation";
const string Normalizer::THRESHOLD = "threshold";
const string Normalizer::Y_ADD = "y_add";
const string Normalizer::Y_MIN = "y_min";
const string Normalizer::Y_MAX = "y_max";
const string Normalizer::LOG = "log";
const string Normalizer::DEFAULT_VALUE = "default";

bool Normalizer::parse(const string &description) {
    if (description.empty()) {
        return true;
    }
    KVPairVec kvPairVec;
    vector<string> keyValueVec;
    StringUtil::fromString(description, keyValueVec, ",");
    for (size_t i = 0; i < keyValueVec.size(); ++i) {
        string key;
        string value;
        StringUtil::getKVValue(keyValueVec[i], key, value, "=", true);
        if (key == METHOD) {
            if (value == "minmax") {
                _type = MINMAX;
            } else if (value == "zscore") { 
                _type = ZSCORE;
            } else if (value == "log10") {
                _type = LOG10;
            } else if (value == "bound_regular") {
                _type = BOUND_REGULAR;
            }
        } else {
            kvPairVec.push_back(make_pair(key, value));
        }
    }
    if (!parseParam(kvPairVec)) {
        AUTIL_LOG(ERROR, "parse description[%s] failed", description.c_str());
        return false;
    }
    return true;
}

bool Normalizer::parseParam(const KVPairVec &kvPairVec) {
    switch (_type) {
    case MINMAX:
        return parseMinMax(kvPairVec);
    case ZSCORE:
        return parseZscore(kvPairVec);
    case LOG10:
        return parseLog10(kvPairVec);
    case BOUND_REGULAR:
        return parseBoundRegular(kvPairVec);
    default:
        return false;
    }
    return false;
}

template<typename T>
bool Normalizer::setParam(const KVPairVec &kvPairVec, 
                          const string &key, T &param) {
    for (size_t i = 0; i < kvPairVec.size(); ++i) {
        if (kvPairVec[i].first == key) {
            return StringUtil::fromString(kvPairVec[i].second, param);
        }
    }
    return false;
}

bool Normalizer::parseMinMax(const KVPairVec &kvPairVec) {
    if (kvPairVec.size() != 2) {
        AUTIL_LOG(ERROR, "invalid param number.");
        return false;
    }
    if (!setParam(kvPairVec, MIN, _param1) ||
        !setParam(kvPairVec, MAX, _param2)) {
        AUTIL_LOG(ERROR, "parse param failed");
        return false;
    }
    if (_param1 >= _param2) {
        AUTIL_LOG(ERROR, "invalid parameters, min[%f] >= max[%f]",
                  _param1, _param2);
        return false;
    }
    return true;
}

bool Normalizer::parseZscore(const KVPairVec &kvPairVec) {
    if (kvPairVec.size() != 2) {
        return false;
    }
    if (!setParam(kvPairVec, MEAN, _param1) ||
        !setParam(kvPairVec, STANDARD_DEVIATION, _param2)) {
        AUTIL_LOG(ERROR, "parse param failed");
        return false;
    }
    if (_param2 == 0.0) {
        AUTIL_LOG(ERROR, "standard_deviation is 0.0");
        return false;
    }
    return true;
}

bool Normalizer::parseLog10(const KVPairVec &kvPairVec) {
    if (kvPairVec.size() > 2) {
        return false;
    }
    _param1 = 1.0;
    _param2 = 0.0;
    if (kvPairVec.size() == 1) {
        return (setParam(kvPairVec, THRESHOLD, _param1)
                || setParam(kvPairVec, DEFAULT_VALUE, _param2))
                && _param1 > 0;
    } else if (kvPairVec.size() == 2) {
        return setParam(kvPairVec, THRESHOLD, _param1)
            && setParam(kvPairVec, DEFAULT_VALUE, _param2)
            && _param1 > 0;
    } else {
        return true;
    }
}

bool Normalizer::parseBoundRegular(const KVPairVec &kvPairVec) {
    if (kvPairVec.size() != 6) {
        return false;
    }

#define SET_PARAM(K, V) if (!setParam(kvPairVec, K, V)) {       \
        AUTIL_LOG(ERROR, "parse param [%s] failed", K.c_str()); \
        return false;                                           \
    }
    SET_PARAM(MEAN, _param1);
    SET_PARAM(STANDARD_DEVIATION, _param2);
    SET_PARAM(Y_ADD, _param3);
    SET_PARAM(Y_MIN, _param4);
    SET_PARAM(Y_MAX, _param5);
    SET_PARAM(LOG, _switch1);
#undef SET_PARAM

    if (_param2 == 0.0) {
        AUTIL_LOG(ERROR, "standard_deviation is 0.0");
        return false;
    }
    return true;    
}

}
