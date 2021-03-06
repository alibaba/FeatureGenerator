#include "fg_lite/feature/Base64.h"

namespace fg_lite {
static const char basis64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64::Encode(unsigned char *dst, const int &dsize,
               const unsigned char *src, const int &size) {
    // check the output buffer size
    int len = size / 3;
    int encode_len = 0;
    if (size % 3) {
        len++;
    }
    if (len * 4  >= dsize) {
        return -1;
    }

    len = size;
    const unsigned char *s = src;
    unsigned char *d = dst;
    while (len > 2) {
        *d++ = basis64[(s[0] >> 2) & 0x3f];
        *d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
        *d++ = basis64[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        *d++ = basis64[s[2] & 0x3f];
        s += 3;
        len -= 3;
        encode_len += 4;
    }

    if (len) {
        *d++ = basis64[(s[0] >> 2) & 0x3f];
        encode_len += 1;

        if (len == 1) {
            *d++ = basis64[(s[0] & 3) << 4];
            *d++ = '=';
            encode_len += 2;

        } else {
            *d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
            *d++ = basis64[(s[1] & 0x0f) << 2];
            encode_len += 2;
        }
        *d++ = '=';
        encode_len += 1;
    }
    *d = 0;
    return encode_len;
}

int Base64::Encode(std::string *dst,  const std::string &src) {
    *dst = "";
    if (src.empty()) {
        return 0;
    }
    int size = 16;
    if (4 < src.length() * 2 + 1) {
        size =  src.length() * 2 + 1;
    }
    unsigned char *buffer = new unsigned char[size];
    if (NULL == buffer) {
        return -1;
    }
    const unsigned char *s = reinterpret_cast<const unsigned char *>(src.c_str());
    int len = Encode(buffer, size, s, src.length());
    if (len > 0) {
        *dst = std::string(reinterpret_cast<char *>(buffer));
    }
    delete [] buffer;
    buffer = NULL;
    return len;
}

static int DecodeInternal(unsigned char *dst, const int &dsize,
                               const unsigned char *src, const int &size, const unsigned char *basis) {
    // length(src) * 3/4 < length(dst);
    if (size == 0) {
        *dst = 0;
        return 0;
    }
    if (size < 4) {
        return -1;
    }
    if (size == 4 && dsize < 4) {
        return -1;
    }
    if (size > 4 && size * 3 >= dsize * 4) {
        return -1;
    }
    int len;

    for (len = 0; len < size; len++) {
        if (src[len] == '=') {
            break;
        }
        if (basis[src[len]] == 77) {
            return -1;
        }
    }

    if (len % 4 == 1) {
        return -1;
    }

    const unsigned char *s = src;
    unsigned char *d = dst;
    int decode_len = 0;
    while (len > 3) {
        *d++ = (unsigned char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        *d++ = (unsigned char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        *d++ = (unsigned char) (basis[s[2]] << 6 | basis[s[3]]);
        s += 4;
        len -= 4;
        decode_len += 3;
    }

    if (len > 1) {
        *d++ = (unsigned char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        decode_len += 1;
    }

    if (len > 2) {
        *d++ = (unsigned char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        decode_len += 1;
    }

    *d = 0;
    return decode_len;
}

int Base64::Decode(unsigned char *dst, const int &dsize,
               const unsigned char *src, const int &size) {
    static unsigned char basis64[] = {
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
            77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
            77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };
    return DecodeInternal(dst, dsize, src, size, basis64);
}

int Base64::Decode(std::string *dst, const std::string &src) {
    if (src.empty()) {
        return 0;
    }
    unsigned char *buffer = new unsigned char[src.length()];
    if (NULL == buffer) {
        return -1;
    }
    const unsigned char *s = reinterpret_cast<const unsigned char *>(src.c_str());
    int len = Decode(buffer, src.length(), s, src.length());
    if (len > 0) {
        *dst = std::string(reinterpret_cast<char *>(buffer), len);
    }
    delete [] buffer;
    buffer = NULL;
    return len;
}
}
