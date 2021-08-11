#include "autil/StringTokenizer.h"
#include "autil/Log.h"

using namespace std;
namespace autil {
AUTIL_DECLARE_AND_SETUP_LOGGER(autil, StringTokenizer);

StringTokenizer::StringTokenizer(const string& str,
                                 const string& sep, Option opt) {
    tokenize(str, sep, opt);
}

StringTokenizer::StringTokenizer() {
}

StringTokenizer::~StringTokenizer() {
}

size_t StringTokenizer::tokenize(const string& str,
                                 const string& sep, Option opt) {
    m_tokens = tokenize(ConstString(str), sep, opt);
    return m_tokens.size();
}

std::vector<ConstString> StringTokenizer::constTokenize(
        const ConstString& str, const std::string& sep, Option opt)
{
    vector<ConstString> ret;
    tokenize(str, sep, ret, opt);
    return ret;
}

std::vector<ConstString> StringTokenizer::constTokenize(
        const ConstString& str, char c, Option opt)
{
    vector<ConstString> ret;
    tokenize(str, c, ret, opt);
    return ret;
}

std::vector<std::string> StringTokenizer::tokenize(
        const ConstString& str, const std::string &sep, Option opt)
{
    vector<string> ret;
    tokenize(str, sep, ret, opt);
    return ret;
}

std::vector<std::string> StringTokenizer::tokenize(
        const ConstString& str, char c, Option opt)
{
    vector<string> ret;
    tokenize(str, c, ret, opt);
    return ret;
}

}
