#include "autil/CountedMultiValueType.h"

using namespace std;

namespace autil {

std::ostream& operator <<(std::ostream& stream, CountedMultiInt8 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << (int)value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiUInt8 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << (int)value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiInt16 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiUInt16 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiInt32 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiUInt32 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiInt64 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiUInt64 value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiFloat value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiDouble value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiChar value)
{
    return stream << string(value.data(), value.size());
}

std::ostream& operator <<(std::ostream& stream, CountedMultiString value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, CountedMultiValueType<autil::uint128_t> value) {
    stream << "size=" << value.size() << ",data=[";
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i] << MULTI_VALUE_DELIMITER;
    }
    stream << "]";
    return stream;
}

}

