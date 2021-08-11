#pragma once
#include "autil/bitmap.h"
#include "autil/ConstString.h"
#include "autil/MurmurHash.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

namespace fg_lite {

template <typename KeyType, typename ValueType>
class SparseFormatter {
    // format
    // keyNum | key1, key2,...,keyN | offset1,...,offsetN | <bitmap1,values1>, ..., <bitmapN, valuesN>
    // 4bytes | sizeof(key) * N     | 4 * N               |
    // offset1 = 0, offset2 = bitmap1_bytes + sizeof(ValueType) * len(values1), ...
public:
    typedef uint32_t KeyNumType;
    typedef uint32_t OffsetType;

    static constexpr size_t KEY_NUM_BYTES = sizeof(KeyNumType);
    static constexpr size_t OFFSET_BYTES = sizeof(OffsetType);
    static constexpr size_t KEY_BYTES = sizeof(KeyType);
    static constexpr size_t VALUE_BYTES = sizeof(ValueType);
public:
    SparseFormatter(const char *data, uint32_t dimension)
        : _data(data)
        , _dimension(dimension)
    {
        _keyNum = *(uint32_t *)_data;
        _keys = (KeyType *)(_data + 4);
        _offsets = (uint32_t *)(_keys + _keyNum);
        _values = (uint8_t *)(_offsets + _keyNum);
    }
public:
    KeyType *getKeys() const {
        return _keys;
    }
    uint32_t *getOffsets() const {
        return _offsets;
    }
    uint8_t *getValues() const {
        return _values;
    }
    size_t encodeValue(uint8_t *addr, const std::vector<float> &values) {
        auto slotCount = autil::Bitmap::GetSlotCount(_dimension);
        autil::Bitmap bitmap((uint32_t *)addr, _dimension, slotCount);
        auto bitmapBytes = slotCount * autil::Bitmap::SLOT_BYTES;
        ValueType *valueBase = (ValueType *)(addr + bitmapBytes);
        uint32_t idx = 0;
        for (size_t i = 0; i < std::min(values.size(), (size_t)_dimension); ++i) {
            if (values[i] != 0) {
                bitmap.Set(i);
                valueBase[idx++] = (ValueType)values[i];
            }
        }
        return bitmapBytes + idx * sizeof(ValueType);
    }
    std::pair<bool, uint32_t> find(KeyType key) const {
        KeyType *end = _keys + _keyNum;
        auto it = std::lower_bound(_keys, end, key);
        if (it != end && *it == key) {
            return std::make_pair(true, it - _keys);
        }
        return std::make_pair(false, (uint32_t)-1);
    }
    void decode(uint32_t keyIdx, float *buffer, uint32_t bufferSize) const {
        assert(keyIdx < _keyNum);
        uint32_t valueOffset = _offsets[keyIdx];
        uint8_t *valueAddr = _values + valueOffset;
        decodeValue(valueAddr, buffer, bufferSize);
    }
private:
    void decodeValue(uint8_t *valueAddr, float *buffer, uint32_t bufferSize) const {
        auto slotCount = autil::Bitmap::GetSlotCount(_dimension);
        autil::Bitmap bitmap((uint32_t *)valueAddr, _dimension, slotCount);
        auto bitmapBytes = slotCount * autil::Bitmap::SLOT_BYTES;
        ValueType *values = (ValueType *)(valueAddr + bitmapBytes);
        uint32_t valueIdx = 0;
        for (uint32_t i = 0; i < _dimension; ++i) {
            if (bitmap.Test(i)) {
                buffer[i] = (float)values[valueIdx++];
            } else {
                buffer[i] = 0.0f;
            }
        }
    }
public:
    static size_t getEncodedLength(uint32_t keyNum, uint32_t valueNum, uint32_t dimension) {
        auto bitmapSlotCount = autil::Bitmap::GetSlotCount(dimension);
        size_t bitmapSize = bitmapSlotCount * autil::Bitmap::SLOT_BYTES;
        size_t perKeySize = KEY_BYTES + bitmapSize  + OFFSET_BYTES;
        size_t totalBytes = KEY_NUM_BYTES + perKeySize * keyNum + valueNum * VALUE_BYTES;
        return totalBytes;
    }
private:
    const char *_data;
    uint32_t _dimension;
    uint32_t _keyNum;
    KeyType *_keys;
    uint32_t *_offsets;
    uint8_t *_values;
};

template <typename KeyType, typename ValueType>
class SparseEncoder {
public:
    using TypedSparseFormatter = SparseFormatter<KeyType, ValueType>;
public:
    SparseEncoder(uint32_t keyNum, uint32_t valueNum, uint32_t dimension) {
        _encodeBuffer.resize(TypedSparseFormatter::getEncodedLength(keyNum, valueNum, dimension), 0);
        *(uint32_t *)_encodeBuffer.data() = keyNum;
        _formatter.reset(new SparseFormatter<KeyType, ValueType>(_encodeBuffer.data(), dimension));
        _currentKey = _formatter->getKeys();
        _currentOffset = _formatter->getOffsets();
        _currentValue = _formatter->getValues();
        _encodedValueSize = 0;
    }
public:
    void encode(const KeyType key, const std::vector<float> &values) {
        *_currentKey++ = key;
        *_currentOffset++ = _encodedValueSize;
        size_t bytes = _formatter->encodeValue(_currentValue, values);
        _encodedValueSize += bytes;
        _currentValue += bytes;
    }
    std::string &getEncodeBuffer() {
        return _encodeBuffer;
    }
private:
    std::string _encodeBuffer;
    std::unique_ptr<TypedSparseFormatter> _formatter;
    // state for encode
    KeyType *_currentKey;
    uint32_t *_currentOffset;
    uint8_t *_currentValue;
    uint32_t _encodedValueSize;
};

template <typename KeyType, typename ValueType>
class SparseDecoder {
    using TypedSparseFormatter = SparseFormatter<KeyType, ValueType>;
public:
    SparseDecoder(const char *data, uint32_t dimension)
        : _formatter(data, dimension)
    {
    }
public:
    bool find(KeyType key, float *ret, uint32_t dimension) {
        auto it = _formatter.find(key);
        if (!it.first) {
            return false;
        }
        _formatter.decode(it.second, ret, dimension);
        return true;
    }
private:
    TypedSparseFormatter _formatter;
};


class LookupFeatureSparseEncoder {
public:
    LookupFeatureSparseEncoder() = default;
    ~LookupFeatureSparseEncoder() = default;

private:
    LookupFeatureSparseEncoder(const LookupFeatureSparseEncoder &) = delete;
    LookupFeatureSparseEncoder& operator=(const LookupFeatureSparseEncoder &) = delete;

public:
    static bool encode(const std::map<autil::ConstString, std::vector<float>> &docs,
                       const uint32_t dim,
                       std::string &output,
                       LookupFeatureV3KeyType hashType,
                       LookupFeatureV3ValueType valueType);
private:
    static size_t nonZeroValueCount(const std::vector<float> &values, size_t dim) {
        size_t count = 0;
        auto pred = [&count](float v) { if (v != 0) count++; };
        auto begin = values.begin();
        auto end = begin + std::min(values.size(), dim);
        std::for_each(begin, end, std::move(pred));
        return count;
    }
    template <typename KeyType = uint64_t, typename ValueType = float>
    static bool encodeSparse(const std::map<KeyType,std::vector<float>> &values,
                             size_t dim,
                             std::string &output)
    {
        if (values.empty()) {
            output.clear();
            return true;
        }
        size_t valueCount = 0;
        for (const auto& keyVec: values) {
            const auto& vec = keyVec.second;
            valueCount += std::min(dim, nonZeroValueCount(vec, dim));
        }
        SparseEncoder<KeyType, ValueType> encoder(values.size(), valueCount, dim);
        for (const auto &it : values) {
            encoder.encode(it.first, it.second);
        }
        std::string &encodedBuffer = encoder.getEncodeBuffer();
        output.swap(encodedBuffer);
        return true;
    }
protected:
    AUTIL_LOG_DECLARE();
};

} // namespace fg_lite
