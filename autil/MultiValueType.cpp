#include "autil/MultiValueType.h"
#include <iomanip>

using namespace std;

namespace autil {

std::ostream& stringMultiTypeStream(std::ostream& stream, MultiChar value) {
    stream << string(value.data(), value.size());
    return stream;
}

template<typename T>
std::ostream& normalMultiTypeStream(std::ostream& stream, T value) {
    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            stream << MULTI_VALUE_DELIMITER;
        }
        stream << value[i];
    }
    return stream;
}

template<typename T>
std::ostream& intMultiTypeStream(std::ostream& stream, T value) {
    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            stream << MULTI_VALUE_DELIMITER;
        }
        stream << (int)value[i];
    }
    return stream;
}

template<typename T>
std::ostream& floatMultiTypeStream(std::ostream& stream, T value) {
    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            stream << MULTI_VALUE_DELIMITER;
        }
        stream << setprecision(6) << value[i];
    }
    return stream;
}

template<typename T>
std::ostream& doubleMultiTypeStream(std::ostream& stream, T value) {
    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            stream << MULTI_VALUE_DELIMITER;
        }
        stream << setprecision(15) << value[i];
    }
    return stream;
}

#define STREAM_FUNC(func, type)                                         \
    std::ostream& operator <<(std::ostream& stream, type value) {       \
        return func(stream, value);                                     \
    }

STREAM_FUNC(intMultiTypeStream, MultiUInt8);
STREAM_FUNC(intMultiTypeStream, MultiInt8);
STREAM_FUNC(normalMultiTypeStream, MultiInt16);
STREAM_FUNC(normalMultiTypeStream, MultiUInt16);
STREAM_FUNC(normalMultiTypeStream, MultiInt32);
STREAM_FUNC(normalMultiTypeStream, MultiUInt32);
STREAM_FUNC(normalMultiTypeStream, MultiInt64);
STREAM_FUNC(normalMultiTypeStream, MultiUInt64);
STREAM_FUNC(normalMultiTypeStream, MultiUInt128);
STREAM_FUNC(floatMultiTypeStream, MultiFloat);
STREAM_FUNC(doubleMultiTypeStream, MultiDouble);
STREAM_FUNC(stringMultiTypeStream, MultiChar);
STREAM_FUNC(normalMultiTypeStream, MultiString);

}
