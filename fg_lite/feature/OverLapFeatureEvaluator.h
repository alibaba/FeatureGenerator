#ifndef ISEARCH_FG_LITE_OVERLAPFEATUREEVALUATOR_H
#define ISEARCH_FG_LITE_OVERLAPFEATUREEVALUATOR_H

#include "fg_lite/feature/FeatureInput.h"
#include "fg_lite/feature/Feature.h"
#include "fg_lite/feature/FeatureFormatter.h"
#include "fg_lite/feature/FeatureConfig.h"
#include "autil/Log.h"
#include "autil/StringUtil.h"


namespace fg_lite {

class OverLapFeatureEvaluator
{
private:
    class ScopedBuffer {
    public:
        ScopedBuffer(FeatureFormatter::FeatureBuffer &buffer)
            : _buffer(buffer)
            , _pos(buffer.size())
        {}
        ~ScopedBuffer() {
            _buffer.erase(_buffer.begin() + _pos, _buffer.end());
        }
    private:
        FeatureFormatter::FeatureBuffer &_buffer;
        size_t _pos;
    };
public:
    OverLapFeatureEvaluator(const std::string &featureName,
                            const std::string &separator,
                            const OverLapFeatureConfig::OverLapType &type,
                            int cutThreshold = -1);
private:
    OverLapFeatureEvaluator(const OverLapFeatureEvaluator &);
    OverLapFeatureEvaluator& operator=(const OverLapFeatureEvaluator &);
public:
    template <typename QT, typename TT, typename CT = TT>
    void genFeature(const Row<QT> &queryList,
                    const Row<TT> &titleList,
                    SingleDenseFeatures *features,
                    const Row<CT> * /*comboList = nullptr*/) const
    {
        int size = 0;
        std::vector<QT> results;
        switch (_type) {
        case OverLapFeatureConfig::OT_EQUAL:
            features->addFeatureValue(isEqual<QT, TT>(queryList, titleList));
            break;
        case OverLapFeatureConfig::OT_CONTAIN:
            features->addFeatureValue(isContain<QT, TT>(queryList, titleList));
            break;
        case OverLapFeatureConfig::OT_COMMON_WORDS:
            intersect<QT, TT>(queryList, titleList, results, false);
            size = int(results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            features->addFeatureValue(size);
            break;
        case OverLapFeatureConfig::OT_HIT_ORNOT:
            intersect<QT, TT>(queryList, titleList, results, false);
            size = int(results.size()) == 0 ? 0 : 1;
            features->addFeatureValue(size);
            break;
        case OverLapFeatureConfig::OT_MATCH_WORDS:
            match<QT, TT>(queryList, titleList, results);
            size = int(results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            features->addFeatureValue(size);
            break;
        case OverLapFeatureConfig::OT_DIFF_WORDS:
            intersect<QT, TT>(queryList, titleList, results, true);
            size = int(results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            features->addFeatureValue(size);
            break;
        case OverLapFeatureConfig::OT_DIFF_BOTH:
            intersect<QT, TT>(queryList, titleList, results, false);
            size = int(queryList.size() + titleList.size() - 2 * results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            features->addFeatureValue(size);
            break;
        case OverLapFeatureConfig::OT_QUERY_RATIO:
            features->addFeatureValue(ratioOp<QT, TT>(queryList, titleList, 10, queryList.size()));
            break;
        case OverLapFeatureConfig::OT_TITLE_RATIO:
            features->addFeatureValue(ratioOp<QT, TT>(queryList, titleList, 100, titleList.size()));
            break;
        default:
            AUTIL_LOG(ERROR, "unsupport type %d", int(_type));
            break;
        }
    }

    template <typename QT, typename TT, typename CT = TT>
    void genFeature(const Row<QT> &queryList,
                    const Row<TT> &titleList,
                    SingleSparseFeatures *features,
                    const Row<CT> * comboList = nullptr) const
    {
        int size = 0;
        std::vector<QT> results;
        switch (_type) {
        case OverLapFeatureConfig::OT_COMMON_WORDS: {
            intersect<QT, TT>(queryList, titleList, results, false);
            ScopedBuffer buffer(_buffer);
            size = int(results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            FeatureFormatter::fillFeatureToBuffer(size, _buffer);
            if (comboList != nullptr) {
                FeatureFormatter::fillFeatureToBuffer(_separator, _buffer);
                FeatureFormatter::fillFeatureToBuffer((*comboList)[0], _buffer);
            }
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        case OverLapFeatureConfig::OT_HIT_ORNOT: {
            intersect<QT, TT>(queryList, titleList, results, false);
            ScopedBuffer buffer(_buffer);
            size = int(results.size()) == 0 ? 0 : 1;
            FeatureFormatter::fillFeatureToBuffer(size, _buffer);
            if (comboList != nullptr) {
                FeatureFormatter::fillFeatureToBuffer(_separator, _buffer);
                FeatureFormatter::fillFeatureToBuffer((*comboList)[0], _buffer);
            }
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        case OverLapFeatureConfig::OT_MATCH_WORDS: {
            match<QT, TT>(queryList, titleList, results);
            ScopedBuffer buffer(_buffer);
            size = int(results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            FeatureFormatter::fillFeatureToBuffer(size, _buffer);
            if (comboList != nullptr) {
                FeatureFormatter::fillFeatureToBuffer(_separator, _buffer);
                FeatureFormatter::fillFeatureToBuffer((*comboList)[0], _buffer);
            }
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        case OverLapFeatureConfig::OT_DIFF_WORDS: {
            intersect<QT, TT>(queryList, titleList, results, true);
            ScopedBuffer buffer(_buffer);
            size = int(results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            FeatureFormatter::fillFeatureToBuffer(size, _buffer);
            if (comboList != nullptr) {
                FeatureFormatter::fillFeatureToBuffer(_separator, _buffer);
                FeatureFormatter::fillFeatureToBuffer((*comboList)[0],  _buffer);
            }
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        case OverLapFeatureConfig::OT_DIFF_BOTH: {
            intersect<QT, TT>(queryList, titleList, results, false);
            ScopedBuffer buffer(_buffer);
            size = int(queryList.size() + titleList.size() - 2 * results.size());
            if (_cutThreshold > 0 && size > _cutThreshold) {
                size = _cutThreshold;
            }
            FeatureFormatter::fillFeatureToBuffer(size, _buffer);
            if (comboList != nullptr) {
                FeatureFormatter::fillFeatureToBuffer(_separator, _buffer);
                FeatureFormatter::fillFeatureToBuffer((*comboList)[0], _buffer);
            }
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        default:
            AUTIL_LOG(ERROR, "unsupport type %d", int(_type));
            break;
        }
    }

    template <typename QT, typename TT, typename CT = TT>
    void genFeature(const Row<QT> &queryList,
                    const Row<TT> &titleList,
                    MultiSparseFeatures *features,
                    const Row<CT> *comboList = nullptr) const
    {
        size_t feature_size_before = features->getFeatures().size();
        switch (_type) {
        case OverLapFeatureConfig::OT_EQUAL:
        {
            bool flag = isEqual<QT, TT>(queryList, titleList);
            ScopedBuffer b(_buffer);
            _buffer.push_back(flag ? '1' : '0');
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        case OverLapFeatureConfig::OT_CONTAIN:
        {
            bool flag = isContain<QT, TT>(queryList, titleList);
            ScopedBuffer b(_buffer);
            _buffer.push_back(flag ? '1' : '0');
            features->addFeatureKey(_buffer.data(), _buffer.size());
            break;
        }
        case OverLapFeatureConfig::OT_COMMON_WORDS:
            setOp<QT, TT>(queryList, titleList, features, false, false);
            break;
        case OverLapFeatureConfig::OT_MATCH_WORDS:
            setOp<QT, TT>(queryList, titleList, features, false, false, true);
            break;
        case OverLapFeatureConfig::OT_MATCH_WORDS_DIVIDED:
            setOp<QT, TT>(queryList, titleList, features, false, true, true);
            break;
        case OverLapFeatureConfig::OT_DIFF_WORDS:
            setOp<QT, TT>(queryList, titleList, features, true, false);
            break;
        case OverLapFeatureConfig::OT_COMMON_WORDS_DIVIDED:
            setOp<QT, TT>(queryList, titleList, features, false, true);
            break;
        case OverLapFeatureConfig::OT_DIFF_WORDS_DIVIDED:
            setOp<QT, TT>(queryList, titleList, features, true, true);
            break;
        case OverLapFeatureConfig::OT_QUERY_RATIO:
        {
            if (queryList.size() > 0) {
                int res = ratioOp<QT, TT>(queryList, titleList, 10, queryList.size());
                ScopedBuffer b(_buffer);
                FeatureFormatter::fillFeatureToBuffer<uint32_t>(res, _buffer);
                features->addFeatureKey(_buffer.data(), _buffer.size());
            }
            break;
        }
        case OverLapFeatureConfig::OT_TITLE_RATIO:
        {
            if (titleList.size() > 0) {
                int res = ratioOp<QT, TT>(queryList, titleList, 100, titleList.size());
                ScopedBuffer b(_buffer);
                FeatureFormatter::fillFeatureToBuffer<uint32_t>(res, _buffer);
                features->addFeatureKey(_buffer.data(), _buffer.size());
            }
            break;
        }
        default:
            AUTIL_LOG(ERROR, "unsupport type %d", int(_type));
            break;
        }

        // 和当前ad的叶子类目id交叉！
        if (comboList != nullptr) {
            size_t feature_size_after = features->getFeatures().size();
            auto& features_cur = features->_featureNames;
            auto pool_cur = features->getPool();
            for (size_t i = feature_size_before; i < feature_size_after; ++i) {
                ScopedBuffer buffer(_buffer);
                size_t cur_bufsz = _buffer.size();
                FeatureFormatter::fillFeatureToBuffer(features_cur[i].toString(), _buffer);
                FeatureFormatter::fillFeatureToBuffer(_separator, _buffer);
                FeatureFormatter::fillFeatureToBuffer((*comboList)[0], _buffer);
                size_t last_bufsz = _buffer.size();
                features_cur[i] = autil::ConstString(_buffer.data() + cur_bufsz, last_bufsz - cur_bufsz, pool_cur);
            }
        }
    }
private:
    template <typename QT, typename TT>
    bool isEqual(const Row<QT> &queryList,
                 const Row<TT> &titleList) const
    {
        if (queryList.size() != titleList.size()) {
            return false;
        }
        for (size_t i = 0; i < queryList.size(); i++) {
            if (!isSame<QT, TT>(queryList[i], titleList[i])) {
                return false;
            }
        }
        return true;
    }

    template <typename QT, typename TT>
    bool isContain(const Row<QT> &queryList,
                   const Row<TT> &titleList) const
    {
        bool flag = false;
        for (size_t i = 0; i + queryList.size() <= titleList.size(); i++) {
            flag = true;
            for (size_t j = 0; j < queryList.size(); j++) {
                if (!isSame<QT, TT>(queryList[j], titleList[i + j])) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                break;
            }
        }
        return flag;
    }

    template <typename QT, typename TT>
    void setOp(const Row<QT> &queryList,
               const Row<TT> &titleList,
               MultiSparseFeatures *features,
               bool diff,
               bool divided,
               bool useMatch = false) const
    {
        std::vector<QT> results;
        if (useMatch && !diff) {
            match<QT, TT>(queryList, titleList, results);
        } else {
            intersect<QT, TT>(queryList, titleList, results, diff);
        }
        if (divided == true) {
            for (size_t i = 0; i < results.size(); i++) {
                ScopedBuffer b(_buffer);
                FeatureFormatter::fillFeatureToBuffer<QT>(results[i], _buffer);
                features->addFeatureKey(_buffer.data(), _buffer.size());
            }
            return;
        }
        ScopedBuffer c(_buffer);
        for (size_t i = 0; i < results.size(); i++) {
            FeatureFormatter::fillFeatureToBuffer<QT>(results[i], _buffer);
            if (i != results.size() - 1) {
                _buffer.insert(_buffer.end(), _separator.begin(), _separator.end());
            }
        }
        if (results.size() > 0) {
            features->addFeatureKey(_buffer.data(), _buffer.size());
        }
    }

    template <typename QT, typename TT>
    uint32_t ratioOp(const Row<QT> &queryList,
                     const Row<TT> &titleList,
                     float multiply, size_t n) const
    {
        if (n == 0) {
            return 0;
        }
        std::vector<QT> results;
        intersect<QT, TT>(queryList, titleList, results, false);
        return multiply * results.size() / n;
    }

    template <typename QT, typename TT>
    void match(const Row<QT> &queryList,
                   const Row<TT> &titleList,
                   std::vector<QT> &results) const
    {
        for (size_t i = 0; i < queryList.size(); i++) {
            for (size_t j = 0 ; j < titleList.size(); j++) {
                if (isSame<QT, TT>(queryList[i], titleList[j])) {
                    results.push_back(queryList[i]);
                }
            }
        }
    }

    template <typename QT, typename TT>
    void intersect(const Row<QT> &queryList,
                   const Row<TT> &titleList,
                   std::vector<QT> &results,
                   bool reverse) const
    {
        for (size_t i = 0; i < queryList.size(); i++) {
            bool found = false;
            for (size_t j = 0 ; j < titleList.size(); j++) {
                if (isSame<QT, TT>(queryList[i], titleList[j])) {
                    found = true;
                    break;
                }
            }
            if (found ^ reverse) {
                results.push_back(queryList[i]);
            }
        }
    }

    template <typename QT, typename TT>
    bool isSame(const QT v1, const TT v2) const {
        return autil::StringUtil::toString(v1) == autil::StringUtil::toString(v2);
    }
private:
    const std::string& _separator;
    mutable autil::mem_pool::Pool _pool;
    mutable FeatureFormatter::FeatureBuffer _buffer;
    OverLapFeatureConfig::OverLapType _type;
    int _cutThreshold;
private:
    AUTIL_LOG_DECLARE();
};

template<>
inline bool OverLapFeatureEvaluator::isSame(const int8_t v1, const int8_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const int16_t v1, const int16_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const int32_t v1, const int32_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const int64_t v1, const int64_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const uint8_t v1, const uint8_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const uint16_t v1, const uint16_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const uint32_t v1, const uint32_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const uint64_t v1, const uint64_t v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const float v1, const float v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const double v1, const double v2) const {
    return v1 == v2;
}

template<>
inline bool OverLapFeatureEvaluator::isSame(const autil::ConstString v1,
                                            const autil::ConstString v2) const {
  if (v1.size() != v2.size()) {
    return false;
  }
  return memcmp(v1.data(), v2.data(), v1.size()) == 0;
}

}

#endif //ISEARCH_FG_LITE_OVERLAPFEATUREEVALUATOR_H
