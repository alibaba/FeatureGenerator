#ifndef ISEARCH_FG_LITE_FEATUREINPUT_H
#define ISEARCH_FG_LITE_FEATUREINPUT_H

#include <memory>
#include "fg_lite/feature/FeatureFormatter.h"
#include "autil/MultiValueType.h"

namespace fg_lite {

enum InputDataType {
    IT_INVALID = 0,
    IT_INT8 = 1,
    IT_UINT8 = 2,
    IT_INT16 = 3,
    IT_UINT16 = 4,
    IT_INT32 = 5,
    IT_UINT32 = 6,
    IT_INT64 = 7,
    IT_UINT64 = 8,
    IT_FLOAT = 9,
    IT_DOUBLE = 10,
    IT_STRING = 11, // for autil::MultiChar
    IT_CSTRING = 12, // for std::string
};

#define INT_INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)      \
    MY_MACRO(IT_INT8);                                  \
    MY_MACRO(IT_UINT8);                                 \
    MY_MACRO(IT_INT16);                                 \
    MY_MACRO(IT_UINT16);                                \
    MY_MACRO(IT_INT32);                                 \
    MY_MACRO(IT_UINT32);                                \
    MY_MACRO(IT_INT64);                                 \
    MY_MACRO(IT_UINT64);

#define STRING_INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)       \
    MY_MACRO(IT_STRING);                                    \
    MY_MACRO(IT_CSTRING);

#define NUMERIC_INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)  \
    INT_INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)          \
    MY_MACRO(IT_FLOAT);                                 \
    MY_MACRO(IT_DOUBLE);

#define INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)          \
    NUMERIC_INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)      \
    STRING_INPUT_DATA_TYPE_MACRO_HELPER(MY_MACRO)

#define TWO_INPUT_DATA_TYPE_MACRO_HELPER(T, MY_MACRO)   \
    MY_MACRO(T, IT_INT8);                               \
    MY_MACRO(T, IT_UINT8);                              \
    MY_MACRO(T, IT_INT16);                              \
    MY_MACRO(T, IT_UINT16);                             \
    MY_MACRO(T, IT_INT32);                              \
    MY_MACRO(T, IT_UINT32);                             \
    MY_MACRO(T, IT_INT64);                              \
    MY_MACRO(T, IT_UINT64);                             \
    MY_MACRO(T, IT_STRING);                             \
    MY_MACRO(T, IT_CSTRING);                            \
    MY_MACRO(T, IT_FLOAT);                              \
    MY_MACRO(T, IT_DOUBLE);

#define THREE_INPUT_DATA_TYPE_MACRO_HELPER(T1, T2, MY_MACRO)   \
    MY_MACRO(T1, T2, IT_INT8);                               \
    MY_MACRO(T1, T2, IT_UINT8);                              \
    MY_MACRO(T1, T2, IT_INT16);                              \
    MY_MACRO(T1, T2, IT_UINT16);                             \
    MY_MACRO(T1, T2, IT_INT32);                              \
    MY_MACRO(T1, T2, IT_UINT32);                             \
    MY_MACRO(T1, T2, IT_INT64);                              \
    MY_MACRO(T1, T2, IT_UINT64);                             \
    MY_MACRO(T1, T2, IT_STRING);                             \
    MY_MACRO(T1, T2, IT_CSTRING);                            \
    MY_MACRO(T1, T2, IT_FLOAT);                              \
    MY_MACRO(T1, T2, IT_DOUBLE);

template <typename T>
struct Type2InputType {
    static constexpr InputDataType value = IT_INVALID;
};

template <InputDataType IT>
struct InputType2Type {
    struct UnknownType {};
    typedef UnknownType Type;
};

#define TYPE_TRAITS_HELPER(input_type, type)            \
    template <>                                         \
    struct Type2InputType<type> {                       \
        static constexpr InputDataType value = input_type;      \
    };                                                  \
    template <>                                         \
    struct InputType2Type<input_type> {                 \
        typedef type Type;                              \
    }

TYPE_TRAITS_HELPER(IT_INT8, int8_t);
TYPE_TRAITS_HELPER(IT_INT16, int16_t);
TYPE_TRAITS_HELPER(IT_INT32, int32_t);
TYPE_TRAITS_HELPER(IT_INT64, int64_t);
TYPE_TRAITS_HELPER(IT_UINT8, uint8_t);
TYPE_TRAITS_HELPER(IT_UINT16, uint16_t);
TYPE_TRAITS_HELPER(IT_UINT32, uint32_t);
TYPE_TRAITS_HELPER(IT_UINT64, uint64_t);
TYPE_TRAITS_HELPER(IT_FLOAT, float);
TYPE_TRAITS_HELPER(IT_DOUBLE, double);
TYPE_TRAITS_HELPER(IT_STRING, autil::MultiChar);
TYPE_TRAITS_HELPER(IT_CSTRING, std::string);

enum InputStorageType {
    IST_DENSE,
    IST_SPARSE_MULTI_VALUE,
    IST_SPARSE_VALUE_OFFSET
};

template <typename T>
class Row {
public:
    Row()
        : _values(nullptr)
        , _count(0)
    {}
    Row(const T *values, size_t count)
        : _values(values)
        , _count(count)
    {}
public:
    const T *begin() const { return _values; }
    const T *end() const { return _values + _count; }
    size_t size() const {
        return _count;
    }
    T operator[](size_t id) const {
        assert(id < _count);
        return _values[id];
    }
    bool operator==(const Row<T> &other) const {
        if (this == &other) {
            return true;
        }
        if (size() != other.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); i++) {
            if (_values[i] != other[i]) {
                return false;
            }
        }
        return true;
    }
private:
    const T *_values;
    size_t _count;
};

template <typename T>
class DenseStorage {
public:
    static constexpr InputStorageType STORAGE_ENUM = IST_DENSE;
public:
    DenseStorage()
        : _values(nullptr)
        , _row(0)
        , _col(0)
    {}
    DenseStorage(const T *values, size_t r, size_t c = 1)
        : _values(values)
        , _row(r)
        , _col(c)
    {}
public:
    size_t row() const { return _row; }
    size_t col(size_t /* unused */) const { return _col; }
    T get(size_t r, size_t c) const {
        assert(r < row());
        assert(c < col(r));
        return _values[r*_col+c];
    }
    const T &getRef(size_t r, size_t c) const {
        assert(r < row());
        assert(c < col(r));
        return _values[r*_col+c];
    }
    size_t numElements() const {
        return _row * _col;
    }
    Row<T> getRow(size_t r) const {
        assert(r < row());
        return Row<T>(_values + r * _col, _col);
    }
    bool supportRef() const {
        return true;
    }
private:
    const T *_values;
    const size_t _row;
    const size_t _col;
};

template <typename T>
class MultiValueStorage {
public:
    static constexpr InputStorageType STORAGE_ENUM = IST_SPARSE_MULTI_VALUE;
private:
    using ValueType = autil::MultiValueType<T>;
public:
    MultiValueStorage()
        : _values(nullptr)
        , _count(0)
    {}
    MultiValueStorage(const ValueType *values, size_t count)
        : _values(values)
        , _count(count)
    {}
public:
    size_t row() const { return _count; }
    size_t col(size_t r) const {
        assert(r < row());
        return _values[r].size();
    }
    T get(size_t r, size_t c) const {
        assert(r < row());
        assert(c < col(r));
        return _values[r][c];
    }
    const T& getRef(size_t r, size_t c) const {
        assert(r < row());
        assert(c < col(r));
        return _values[r].getRef(c);
    }
    Row<T> getRow(size_t r) const {
        assert(r < row());
        const ValueType &v = _values[r];
        return Row<T>(v.data(), v.size());
    }
    size_t numElements() const {
        size_t total = 0;
        for (size_t r = 0; r < row(); r++) {
            total += col(r);
        }
        return total;
    };
    bool supportRef() const {
        return true;
    }
private:
    const ValueType *_values;
    const size_t _count;
};

template <>
inline bool MultiValueStorage<autil::MultiChar>::supportRef() const {
    return false;
}

template <>
inline const autil::MultiChar& MultiValueStorage<autil::MultiChar>::getRef(
        size_t r, size_t c) const
{
    // MultiString does not support data()
    static autil::MultiChar mc;
    assert(false);
    return mc;
}

template <>
inline Row<autil::MultiChar> MultiValueStorage<autil::MultiChar>::getRow(size_t r) const {
    // MultiString does not support data()
    assert(false);
    return Row<autil::MultiChar>();
}

template <typename T>
class ValueOffsetStorage {
public:
    static constexpr InputStorageType STORAGE_ENUM = IST_SPARSE_VALUE_OFFSET;
public:
    ValueOffsetStorage()
        : _values(nullptr)
        , _offsets(nullptr)
        , _valueCount(0)
        , _offsetCount(0)
    {}
    ValueOffsetStorage(const T *values, size_t valueCount,
                       const size_t *offsets, size_t offsetCount)
        : _values(values)
        , _offsets(offsets)
        , _valueCount(valueCount)
        , _offsetCount(offsetCount)
    {}
    ValueOffsetStorage(const T *values, size_t valueCount,
                       const size_t *offsets, size_t offsetCount,
                       const std::shared_ptr<std::vector<size_t> > offsetVec)
        : _values(values)
        , _offsets(offsets)
        , _valueCount(valueCount)
        , _offsetCount(offsetCount)
        , _offsetVec(offsetVec)
    {}
public:
    size_t row() const { return _offsetCount; }
    size_t col(size_t r) const {
        assert(r < row());
        if (r + 1 != row()) {
            return _offsets[r+1] - _offsets[r];
        } else {
            return _valueCount - _offsets[r];
        }
    }
    T get(size_t r, size_t c) const {
        assert(r < row());
        assert(c < col(r));
        return _values[_offsets[r]+c];
    }
    const T& getRef(size_t r, size_t c) const {
        assert(r < row());
        assert(c < col(r));
        return _values[_offsets[r]+c];
    }
    size_t numElements() const {
        return _valueCount;
    }
    Row<T> getRow(size_t r) const {
        assert(r < row());
        return Row<T>(_values + _offsets[r], col(r));
    }
    bool supportRef() const {
        return true;
    }
private:
    const T *_values;
    const size_t *_offsets;
    const size_t _valueCount;
    const size_t _offsetCount;
    const std::shared_ptr<std::vector<size_t> > _offsetVec; //hold to extend lifetime for offsets vector
};

class FeatureInput {
public:
    FeatureInput(InputDataType dataType, InputStorageType storageType)
        : _dataType(dataType)
        , _storageType(storageType)
    {}
    virtual ~FeatureInput() = default;
public:
    InputDataType dataType() const {
        return _dataType;
    }
    InputStorageType storageType() const {
        return _storageType;
    }
public:
    virtual size_t row() const = 0;
    virtual size_t col(size_t r) const = 0;
    virtual size_t numElements() const = 0;
    virtual bool toString(size_t r, size_t c, FeatureFormatter::FeatureBuffer &buf,
                          bool check=false) = 0;
private:
    InputDataType _dataType;
    InputStorageType _storageType;
};

template <InputDataType IDT, InputStorageType IST>
struct StorageTypeTraits {
    struct UnknownType {};
    typedef UnknownType StorageType;
};

#define STORAGE_TRAITS_HELPER(input_enum, storage_enum, storage_type)   \
    template <>                                                         \
    struct StorageTypeTraits<input_enum, storage_enum> {                \
        typedef typename InputType2Type<input_enum>::Type Type;         \
        typedef storage_type<Type> StorageType;                         \
    }

#define STORAGE_HELPER(input_enum)                                      \
    STORAGE_TRAITS_HELPER(input_enum, IST_DENSE, DenseStorage);         \
    STORAGE_TRAITS_HELPER(input_enum, IST_SPARSE_VALUE_OFFSET, ValueOffsetStorage); \
    STORAGE_TRAITS_HELPER(input_enum, IST_SPARSE_MULTI_VALUE, MultiValueStorage) \

STORAGE_HELPER(IT_INT8);
STORAGE_HELPER(IT_UINT8);
STORAGE_HELPER(IT_INT16);
STORAGE_HELPER(IT_UINT16);
STORAGE_HELPER(IT_INT32);
STORAGE_HELPER(IT_UINT32);
STORAGE_HELPER(IT_INT64);
STORAGE_HELPER(IT_UINT64);
STORAGE_HELPER(IT_FLOAT);
STORAGE_HELPER(IT_DOUBLE);
STORAGE_HELPER(IT_STRING);
STORAGE_HELPER(IT_CSTRING);

template <typename T, typename StorageType>
class FeatureInputTyped : public FeatureInput {
public:
    typedef T value_type;
public:
    FeatureInputTyped(StorageType storage)
        : FeatureInput(Type2InputType<T>::value, StorageType::STORAGE_ENUM)
        , _storage(storage)
    {}
public:
    size_t row() const override { return _storage.row(); }
    size_t col(size_t r) const override { return _storage.col(r); }
    size_t numElements() const override { return _storage.numElements(); }
    bool toString(size_t r, size_t c, FeatureFormatter::FeatureBuffer &buf,
                  bool check) override
    {
        if (_storage.supportRef()) {
            const T &v = getRef(r, c);
            if (check && FeatureFormatter::isInvalidValue<T>(v)) {
                return false;
            }
            FeatureFormatter::fillFeatureToBuffer(v, buf);
        } else {
            T v = get(r, c);
            if (check && FeatureFormatter::isInvalidValue<T>(v)) {
                return false;
            }
            FeatureFormatter::fillFeatureToBuffer(v, buf);
        }
        return true;
    }
public:
    T get(size_t r, size_t c) const {
        return _storage.get(r, c);
    }
    const T& getRef(size_t r, size_t c) const { return _storage.getRef(r, c); }
    Row<T> getRow(size_t r) const { return _storage.getRow(r); }
private:
    StorageType _storage;
};

template <InputDataType DT, InputStorageType ST>
struct DTST2InputType {
    typedef typename InputType2Type<DT>::Type DataType;
    typedef typename StorageTypeTraits<DT, ST>::StorageType StorageType;
    typedef FeatureInputTyped<DataType, StorageType> InputType;
};

}

#endif //ISEARCH_FG_LITE_FEATUREINPUT_H
