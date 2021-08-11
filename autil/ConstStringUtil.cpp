#include "autil/ConstStringUtil.h"

#include <stdio.h>
#include <algorithm>
#include <iosfwd>

using namespace std;
namespace autil {

std::vector<autil::ConstString> ConstStringUtil::split(const autil::ConstString& text, 
                                                       const std::string &sepStr, 
                                                       bool ignoreEmpty)
{
    std::vector<autil::ConstString> vec;
    split(vec, text, sepStr, ignoreEmpty);
    return vec;
}

void ConstStringUtil::split(std::vector<autil::ConstString> &vec,
                            const autil::ConstString& text, 
                            const char &sepChar, bool ignoreEmpty)
{
    split(vec, text, std::string(1, sepChar), ignoreEmpty);
}

void ConstStringUtil::split(std::vector<autil::ConstString> &vec, 
                            const autil::ConstString& text, 
                            const std::string &sepStr, bool ignoreEmpty)
{
    size_t n = 0, old = 0;
    while (n != std::string::npos)
    {
        n = text.find(sepStr, n);
        if (n != std::string::npos)
        {
            if (!ignoreEmpty || n != old) 
                vec.push_back(text.substr(old, n-old));
            n += sepStr.length();
            old = n;
        }
    }

    if (!ignoreEmpty || old < text.length()) {
        vec.push_back(text.substr(old, text.length() - old));
    }
}

}

