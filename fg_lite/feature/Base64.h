#ifndef ISEARCH_FG_LITE_BASE64_H
#define ISEARCH_FG_LITE_BASE64_H

#include <string>

namespace fg_lite {
class Base64 {
public:
    int Encode(unsigned char *dst, const int &dsize,
                   const unsigned char *src, const int &size);

    int Encode(std::string *dst, const std::string &src);

    int Decode(unsigned char *dst, const int &dsize,
                   const unsigned char *src, const int &size);

    int Decode(std::string *dst, const std::string &src);
};

}

#endif //ISEARCH_FG_LITE_BASE64_H

