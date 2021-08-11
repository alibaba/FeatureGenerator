#pragma once

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <iosfwd>
#include <string>

namespace autil {

class ConstString
{
public:
    typedef std::char_traits<char> traits_type;
public:
    ConstString() : _data(NULL), _size(0) {}
    explicit ConstString(const char *data) : _data(data), _size(strlen(data)) {}
    explicit ConstString(const char *data, size_t size) : _data(data), _size(size) {}
    explicit ConstString(const std::string &str) : _data(str.data()), _size(str.size()) {}

    template <typename Allocator>
    explicit ConstString(const char *data, Allocator *allocator) {
        initAndCopy(data, strlen(data), allocator);
    }

    template <typename Allocator>
    ConstString(const char *data, size_t size, Allocator *allocator) {
        initAndCopy(data, size, allocator);
    }

    template <typename Allocator>
    ConstString(const std::string &str, Allocator *allocator) {
        initAndCopy(str.data(), str.size(), allocator);
    }

    template <typename Allocator>
    ConstString(const ConstString &str, Allocator *allocator) {
        initAndCopy(str.data(), str.size(), allocator);
    }

    ConstString(const ConstString &s) : _data(s._data), _size(s._size) {}
    ConstString& operator= (const ConstString &s) {
        _size = s._size;
        _data = s._data;
        return *this;
    }
    ~ConstString() = default;
public:
    static const size_t npos = size_t(-1);
    static ConstString EMPTY_STRING;
public:
    const char *data() const { return _data; }
    char *data() { return const_cast<char*>(_data); }
    // Fatal error may incur when _data is not end with '\0'
    const char *c_str() const { return _data; }

    bool empty() const { return _size == 0; }

    size_t size() const { return _size; }
    size_t length() const { return _size; }

    const char *begin() const { return _data; }
    const char *end() const { return _data + _size; }

    inline bool operator==(const ConstString &rhs) const;
    inline bool operator!=(const ConstString &rhs) const;
    inline bool operator<(const ConstString &rhs ) const;
    inline bool operator<=(const ConstString &rhs ) const;
    inline bool operator>(const ConstString &rhs ) const;
    inline bool operator>=(const ConstString &rhs ) const;

    inline bool operator==(const std::string &rhs) const {
        return *this == ConstString(rhs);
    }
    inline ConstString subString(size_t begin, size_t len = npos) const;
    inline ConstString substr(size_t begin, size_t len = npos) const {
        return subString(begin, len);
    }

    inline size_t find(const std::string &str, size_t pos = 0) const {
        return find(str.data(), pos, str.size());
    }
    inline size_t find(const ConstString &str, size_t pos = 0) const {
        return find(str.data(), pos, str.size());
    }
    inline size_t find(char c, size_t pos = 0) const {
        size_t ret = std::string::npos;
        if (pos < _size) {
            const char *p = traits_type::find(_data + pos, _size - pos, c);
            if (p) {
                ret = p - _data;
            }
        }
        return ret;
    }

    inline size_t rfind(const std::string &str, size_t pos = size_t(-1)) const {
        return rfind(str.data(), pos, str.size());
    }
    inline size_t rfind(const ConstString &str, size_t pos = size_t(-1)) const {
        return rfind(str.data(), pos, str.size());
    }
    inline size_t rfind(char c, size_t pos = size_t(-1)) const {
        size_t size = _size;
        if (size) {
            if (--size > pos) {
                size = pos;
            }
            const char *data = _data;
            for (++size; size-- > 0;) {
                if (data[size] == c) {
                    return size;
                }
            }
        }
        return std::string::npos;
    }

    int compare(const ConstString &str) const {
        size_t size = this->size();
        size_t osize = str.size();
        size_t len = std::min(size, osize);
        int r = traits_type::compare(begin(), str.begin(), len);
        if (!r) {
            r = size - osize;
        }
        return r;
    }

    std::string toString() const { return std::string(_data, _size); }

private:
    template <typename Allocator>
    void initAndCopy(const char *data, size_t size, Allocator *allocator) {
        // allocate one more byte '\0', but size not change
        // it makes ConstString easier to debug
        // example: data = "abc", size = 3 => _data="abc\0",_size=3
        // then you print _data then show "abc" bur data (withou \0) not
        char *buf = (char*)allocator->allocate(size + 1);
        memcpy(buf, data, size);
        buf[size] = '\0';
        _data = buf;
        _size = size;
    }
    inline size_t find(const char *str, size_t pos, size_t n) const {
        size_t ret = std::string::npos;
        size_t size = _size;
        if (pos + n <= size) {
            const char *p = std::search(_data + pos, _data + size,
                    str, str + n, traits_type::eq);
            if (p != _data + size || n == 0) {
                ret = p - _data;
            }
        }
        return ret;
    }
    inline size_t rfind(const char *str, size_t pos, size_t n) const {
        const size_t size = _size;
        if (n <= size) {
            pos = std::min(size - n, pos);
            const char *data = _data;
            do {
                if (traits_type::compare(data + pos, str, n) == 0) {
                    return pos;
                }
            } while (pos-- > 0);
        }
        return std::string::npos;
    }
public:
    void reset(const char* data, size_t size) {
        _data = data;
        _size = size;
    }
    void reset(char* data, size_t size) {
        _data = data;
        _size = size;
    }
private:
    const char *_data;
    size_t _size;
};

inline bool ConstString::operator==(const ConstString &rhs) const {
    return compare(rhs) == 0;
}

inline bool ConstString::operator!=(const ConstString &rhs) const {
    return compare(rhs) != 0;
}

inline bool ConstString::operator<( const ConstString &rhs ) const {
    return compare(rhs) < 0;
}

inline bool ConstString::operator<=( const ConstString &rhs ) const {
    return compare(rhs) <= 0;
}

inline bool ConstString::operator>( const ConstString &rhs ) const {
    return compare(rhs) > 0;
}

inline bool ConstString::operator>=( const ConstString &rhs ) const {
    return compare(rhs) >= 0;
}

inline ConstString ConstString::subString(size_t begin, size_t len) const {
    if (begin >= _size) {
        return ConstString();
    }
    return ConstString(_data + begin, std::min(_size - begin, len));
}

inline std::ostream& operator<<(std::ostream& os, const ConstString &str) {
    return os << std::string(str.data(), str.size());
}

inline bool operator==(const std::string &lhs, const autil::ConstString &rhs) {
    return rhs == lhs;
}

}
