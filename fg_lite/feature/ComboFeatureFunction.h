#ifndef ISEARCH_FG_LITE_COMBOFEATUREFUNCTION_H
#define ISEARCH_FG_LITE_COMBOFEATUREFUNCTION_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"

namespace fg_lite {

class ComboFeatureFunction : public FeatureFunction
{
public:
    ComboFeatureFunction(const std::string &name, const std::string &prefix,
                         const std::vector<bool> &pruneRight,
                         const std::vector<int> &pruneLimit,
                         size_t inputCount,
                         const bool needSort = false);
private:
    ComboFeatureFunction(const ComboFeatureFunction &);
    ComboFeatureFunction& operator=(const ComboFeatureFunction &);

#define MULTI_SEPARATOR '_'
    class NormalCombo {
    public:
        NormalCombo(const FeatureFormatter::FeatureBuffer &prefix)
            : _buffer(prefix)
        {
        }
        template<typename T, typename StorageType>
        bool collect(FeatureInputTyped<T, StorageType>* typedInput,
                     const size_t r, const size_t c, bool check = true)
        {
            return typedInput->toString(r, c, _buffer, check);
        }
        FeatureFormatter::FeatureBuffer getWholeBuf()
        {
            return _buffer;
        }
        void backTrace(const size_t beginPos) {
            _buffer.assign(_buffer.begin(), _buffer.begin() + beginPos);
        }
        void addSeparator() {
            _buffer.push_back(MULTI_SEPARATOR);
        }
        size_t getBufferLength() const{
            return _buffer.size();
        }
    private:
        FeatureFormatter::FeatureBuffer _buffer;
    };

    class SortedCombo {
    public:
        SortedCombo(const FeatureFormatter::FeatureBuffer &prefix)
            : _buffer(prefix)
            , _pool(1024)
            , _bufferVec(std::vector<FeatureFormatter::FeatureBuffer>())
            , _prefix(prefix)
        {
        }
        template<typename T, typename StorageType>
        bool collect(FeatureInputTyped<T, StorageType>* typedInput,
                     const size_t r, const size_t c, bool check = true) {
            std::string tmp = std::string();
            auto tmpBuf = FeatureFormatter::FeatureBuffer(tmp.begin(), tmp.end(),
                    autil::mem_pool::pool_allocator<char>(&_pool));
            if (!typedInput->toString(r, c, tmpBuf, check)) {
                return false;
            }
            _bufferVec.push_back(tmpBuf);
            return true;
        }
        FeatureFormatter::FeatureBuffer getWholeBuf() {
            _buffer.assign(_prefix.begin(), _prefix.end());
            if (_bufferVec.empty()) {
                return _buffer;
            }
            sort(_bufferVec.begin(), _bufferVec.end());
            _buffer.insert(_buffer.end(), _bufferVec[0].begin(), _bufferVec[0].end());
            for (int i = 1; i < _bufferVec.size(); ++i) {
                _buffer.push_back(MULTI_SEPARATOR);
                _buffer.insert(_buffer.end(), _bufferVec[i].begin(), _bufferVec[i].end());
            }
            return _buffer;
        }
        void backTrace(const size_t beginPos) {
            _bufferVec.pop_back();
        }
        void addSeparator() {
            // do nothing
        }
        size_t getBufferLength() const{
            return _buffer.size();
        }

    private:
        FeatureFormatter::FeatureBuffer _buffer;
        autil::mem_pool::UnsafePool _pool;
        std::vector<FeatureFormatter::FeatureBuffer> _bufferVec;
        FeatureFormatter::FeatureBuffer _prefix;
    };
#undef MULTI_SEPARATOR

public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;
    size_t getInputCount() const override {
        return _inputCount;
    }
private:
    template<typename Combo>
    void genFeatureFast(
            const std::vector<FeatureInput*> &inputs,
            size_t id,
            MultiSparseFeatures *features,
            Combo &combo) const;

    template<typename Combo>
    void genFeatureNormal(
            const std::vector<FeatureInput*> &inputs, size_t docId, size_t featureId,
            MultiSparseFeatures *features,
            Combo &combo) const;

    template <typename T, typename StorageType, typename Combo>
    void appendOneFeature(
            const std::vector<FeatureInput*> &inputs,
            size_t docId,
            size_t featureId,
            MultiSparseFeatures *features,
            Combo& combo) const;
private:
    size_t _inputCount;
    std::vector<bool> _pruneRight;
    std::vector<int32_t> _pruneLimit;
    bool _needSort;
public:

private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_COMBOFEATUREFUNCTION_H
