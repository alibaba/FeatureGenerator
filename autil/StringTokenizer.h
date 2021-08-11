#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <cstddef>
#include <string>
#include <vector>

#include "autil/ConstString.h"

namespace autil {

class StringTokenizer
{
public:
    /// ignore empty tokens
    const static int32_t TOKEN_IGNORE_EMPTY = 1;
    /// remove leading and trailing whitespace from tokens
    const static int32_t TOKEN_TRIM         = 2;
    /// leave the token as it is
    const static int32_t TOKEN_LEAVE_AS     = 4;

    typedef int32_t Option;
    typedef std::vector<std::string> TokenVector;
    typedef TokenVector::const_iterator Iterator;
public:
    StringTokenizer(const std::string& str, const std::string& sep,
                    Option opt = TOKEN_LEAVE_AS);
    StringTokenizer();
    ~StringTokenizer();
public:
    size_t tokenize(const std::string& str, const std::string& sep,
                    Option opt = TOKEN_LEAVE_AS);

    inline Iterator begin() const;
    inline Iterator end() const;

    inline const std::string& operator [] (size_t index) const;

    inline size_t getNumTokens() const;
    inline const TokenVector& getTokenVector() const;

public:
    // new implement begin
    static std::vector<ConstString> constTokenize(
            const ConstString& str, const std::string& sep,
            Option opt = TOKEN_IGNORE_EMPTY | TOKEN_TRIM);

    static std::vector<ConstString> constTokenize(
            const ConstString& str, char c,
            Option opt = TOKEN_IGNORE_EMPTY | TOKEN_TRIM);

    static std::vector<std::string> tokenize(
            const ConstString& str, const std::string &sep,
            Option opt = TOKEN_IGNORE_EMPTY | TOKEN_TRIM);

    static std::vector<std::string> tokenize(
            const ConstString& str, char c,
            Option opt = TOKEN_IGNORE_EMPTY | TOKEN_TRIM);


    template<typename ContainerType>
    static void tokenize(const ConstString& str, char sep, ContainerType &container,
                         Option opt = TOKEN_IGNORE_EMPTY | TOKEN_TRIM)
    {
        return tokenize(str, sep, 1, container, opt);
    }
    template<typename ContainerType>
    static void tokenize(const ConstString& str, const std::string &sep,
                         ContainerType &container,
                         Option opt = TOKEN_IGNORE_EMPTY | TOKEN_TRIM)
    {
        return tokenize(str, sep, sep.length(), container, opt);
    }

private:
    template<typename Sep, typename ContainerType>
    static void tokenize(const ConstString& str, Sep &&sep, size_t sepLen,
                         ContainerType &container, Option opt)
    {
        size_t n = 0, old = 0;
        while (n != std::string::npos) {
            n = str.find(sep, n);
            if (n != std::string::npos) {
                if (n != old) {
                    ConstString subStr(str.data() + old, n - old);
                    addToken(subStr, opt, container);
                } else {
                    ConstString subStr("");
                    addToken(subStr, opt, container);
                }

                n += sepLen;
                old = n;
            }
        }
        ConstString subStr(str.c_str() + old, str.size() - old);
        addToken(subStr, opt, container);
    }
    inline static bool isSpace(char ch);

    template <typename ContainerType>
    static void addToken(const ConstString& token,
                         Option opt, ContainerType &target)
    {
        typedef typename ContainerType::value_type T;
        size_t length = token.size();
        const char * data = token.data();
        if (opt & TOKEN_LEAVE_AS) {
            if ( !(opt & TOKEN_IGNORE_EMPTY)) {
                target.push_back(T(data, length));
            } else if (length > 0) {
                target.push_back(T(data, length));
            }
        } else if (opt & TOKEN_TRIM) {
            size_t n = 0;
            while(n < length && isSpace(data[n])) {
                n++;
            }
            size_t n2 = length;
            while(n2 > n && isSpace(data[n2 - 1])) {
                n2--;
            }
            if (n2 > n) {
                target.push_back(T(data + n, n2 - n));
            } else if ( !(opt & TOKEN_IGNORE_EMPTY)) {
                target.push_back(T());//insert an empty token
            }
        } else if (length > 0) {
            target.push_back(T(data, length));
        }
    }
private:
    StringTokenizer(const StringTokenizer&);
    StringTokenizer& operator = (const StringTokenizer&);
private:
    TokenVector m_tokens;
};

///////////////////////////////////////////////////////////
// inlines
//
inline bool StringTokenizer::isSpace(char ch)
{
    return (ch > 0 && std::isspace(ch));
}

inline StringTokenizer::Iterator StringTokenizer::begin() const
{
    return m_tokens.begin();
}

inline StringTokenizer::Iterator StringTokenizer::end() const
{
    return m_tokens.end();
}

inline const std::string& StringTokenizer::operator [] (std::size_t index) const
{
    assert(index < m_tokens.size());
    return m_tokens[index];
}

inline size_t StringTokenizer::getNumTokens() const
{
    return m_tokens.size();
}

inline const StringTokenizer::TokenVector& StringTokenizer::getTokenVector() const {
    return m_tokens;
}

}

