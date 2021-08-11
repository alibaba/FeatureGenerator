#include <ext/alloc_traits.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "autil/legacy/exception.h"
#include "autil/legacy/string_tools.h"

using namespace std;

namespace autil{ namespace legacy
{

enum StringPatternMode
{
    //paggu://file2_{instance_id2}_any_{range(150, 501)}_any_{["xxx", "yyy"]}.dat"
    RANGE, // {range(150, 501)}
    CONTENT, // {["xxx", "yyy"]}
    PLAIN, //"paggu://file2_", "_any_", ".dat", {instance_id2}
    RANGE_ALIGNED
};



StringPattern::StringPattern() {}

void StringPattern::SetPattern(const string& pattern)
{
    mPattern = pattern;
    Reset();
}

void StringPattern::SetVariableMap(const map<string, string>& variableMap)
{
    mVariableMap = variableMap;
    Reset();
}

string StringPattern::GetPattern() const
{
    return mPattern;
}

map<string, string> StringPattern::GetVariableMap() const
{
    return mVariableMap;
}

vector<string> StringPattern::GetStrings()
{
    vector<string> strings;
    Prepare();
    strings.resize(mSize);
    for (unsigned int i = 0; i < mSize; i++)
    {
        strings[i] = ((*this)[i]);
    }        
    return strings;
}

string StringPattern::operator [](size_t idx)
{
    Prepare();
    if (idx >= mSize)
    {
        AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Variable Input : idx exceed the size");
    }
    string result;
    for (unsigned int i = 0; i < mPatternInfos.size(); i++)
    {
        vector<size_t>& subStringNumber = mPatternInfos[i].subStringNumber;        
        if (idx >= subStringNumber[0])
        {
            idx -= subStringNumber[0];
        }
        else
        {
            vector<vector<string> >& patternResults = mPatternInfos[i].partialPatterns;
            for (unsigned int j = 0; j < patternResults.size(); j++)
            {
                unsigned int k = (idx % subStringNumber[j]) / subStringNumber[j + 1];
                result += patternResults[j][k];
            }
            break;
        }
    }   
    return result; 
}

size_t StringPattern::size()
{
    Prepare();
    return mSize;
}

//Public------------------------------------------------------------------------------Private

void StringPattern::Prepare()
{
    if (!mPatternInfos.empty())
        return;
    vector<string> patternVector;
    patternVector = PatternSplit();
    mPatternInfos.resize(patternVector.size());
    for (unsigned int i = 0; i < patternVector.size(); i++)
    {
        vector<vector<string> >& patternResults = mPatternInfos[i].partialPatterns;
        vector<size_t>& subStringNumber = mPatternInfos[i].subStringNumber;
        patternResults = SplitStringIntoPatterns(patternVector[i]);
        subStringNumber.resize(patternResults.size() + 1);
        subStringNumber.back() = 1;
        for (unsigned int j = patternResults.size(); j > 0; j--)
        {
            subStringNumber[j - 1] = subStringNumber[j] * patternResults[j - 1].size();    
        }
        mSize += subStringNumber[0];
    }
}

void StringPattern::Reset()
{
    mPatternInfos.clear();
    mSize = 0;
}

vector<string> StringPattern::PatternSplit(const char& delim /* = ';' */) const
{
    vector<string> strVector;
    if (!mPattern.empty())
    {
        string::size_type posBegin = 0;
        string::size_type posEnd;
        while (posBegin < mPattern.size())
        {
            while (posBegin < mPattern.size() && mPattern[posBegin] == ' ')
            {
                posBegin++;
            }
            if (posBegin < mPattern.size())
            {
                if ((posEnd = mPattern.find(delim, posBegin)) != string::npos)
                {
                    strVector.push_back(TrimString(mPattern.substr(posBegin, posEnd - posBegin)));
                    posBegin = posEnd + 1;
                } 
                else
                {
                    strVector.push_back(TrimString(mPattern.substr(posBegin, mPattern.size() - posBegin)));
                    posBegin = mPattern.size();
                }
            }
        }
    }
    return strVector;
}

vector<vector<string> > StringPattern::SplitStringIntoPatterns(const string& str) const
{
    string s;
    vector<pair<string, StringPatternMode> > patternResults;
    if (!str.empty())
    {
        string::size_type bePos = 0;
        string::size_type enPos;
        while (bePos < str.size())
        {
            if (bePos < str.size() && (enPos = str.find('{', bePos)) != string::npos)
            {
                s = TrimString(str.substr(bePos, enPos - bePos));
                patternResults.push_back(make_pair(s, PLAIN));
                bePos = enPos + 1;
                if (bePos < str.size() && (enPos = str.find('}', bePos)) != string::npos)
                {
                    s = TrimString(str.substr(bePos, enPos - bePos));
                    bePos = enPos + 1;
                    if (s.find("range(") == 0 && s.find(')') == s.size() - 1)
                    {
                        patternResults.push_back(make_pair(s, RANGE));
                    } 
                    else if (s.find("range_aligned(") == 0 && s.find(')') == s.size() - 1)
                    {
                        patternResults.push_back(make_pair(s, RANGE_ALIGNED));
                    } 
                    else if (s.find('[') == 0 && s.rfind(']') == s.size() - 1)
                    {
                        patternResults.push_back(make_pair(s, CONTENT));
                    } 
                    else if (!s.empty() && s.find("range(") == string::npos
                        && s.find(")") == string::npos
                        && s.find("[") == string::npos
                        && s.find("]") == string::npos)
                    {
                        map<string, string>::const_iterator it = mVariableMap.find(s);
                        if (it != mVariableMap.end())
                        {
                            s = it->second;
                            patternResults.push_back(make_pair(s, PLAIN));
                        } else
                        {
                            AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Variable Input : Could not found in variable map");
                        }
                    } else
                    {
                        AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format : Keyword error in {}");
                    }
                } else
                {
                    AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format : } excepted");
                }
            } else
            {
                s = TrimString(str.substr(bePos, str.size() - bePos));
                patternResults.push_back(make_pair(s, PLAIN));
                break;
            }
        } //while
        vector<vector<string> > resultStrVec;
        for (vector<pair<string, StringPatternMode> >::const_iterator iter = patternResults.begin();
            iter != patternResults.end();
            iter++)
        {
            vector<string> tmpVec;
            switch (iter->second)
            {
                case PLAIN:
                    tmpVec.push_back(iter->first);
                    resultStrVec.push_back(tmpVec);
                    break;
                case RANGE:                     
                    resultStrVec.push_back(RangeSpan(iter->first));
                    break;
                case RANGE_ALIGNED:
                    resultStrVec.push_back(RangeSpanAligned(iter->first));
                    break;
                case CONTENT:
                    resultStrVec.push_back(ContentSpan(iter->first));
                    break;
            }
        }
        return resultStrVec;

    } else
    {
        AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid String Input : pattern string is empty");
    }

}

vector<string> StringPattern::ContentSpan(const string& str) const
{
    vector<string> strVector;
    if (str.empty())
    {
         return strVector;
    }
    vector<string> contentVector = InternalSplit(str, '[', ']');
    for (vector<string>::iterator iter = contentVector.begin();
         iter != contentVector.end();
         iter++)
    {
        strVector.push_back(QuoteStrip(*iter));
    }
    return strVector;
}

vector<string> StringPattern::RangeSpan(const string& str) const
{
    vector<string> strVector;
    if (str.empty())
    {
         return strVector;
    }
    vector<string> rangeVector = InternalSplit(str, '(', ')');
    
    return RangeSpanInternal(rangeVector[0], rangeVector[1]);
}

vector<string> StringPattern::RangeSpanAligned(const string& str) const
{
    vector<string> strVector = RangeSpan(str);
    vector<string> rangeVector = InternalSplit(str, '(', ')');
    
    int width = 0;
    char fill = '0';
    if (rangeVector.size() > 2)
    {
        width = atol(rangeVector[2].c_str());
        if (rangeVector.size() > 3 && rangeVector[3].size() == 1)
        {
            fill = rangeVector[3][0];
        }
    }
    AlignStrings(strVector, width, fill);
    return strVector;
}

vector<string> StringPattern::RangeSpanInternal(
    const string& lo, const string& hi) const
{
    vector<string> strVector;
    size_t i = atol(lo.c_str());
    size_t j = atol(hi.c_str());
    if (j < i)
    {
        AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Range Input");
    }
    for (size_t k = i; k <= j; k++)
    {
        strVector.push_back(ToString(k));
    }
    return strVector;
}

bool StringPattern::AlignStrings(vector<string>& vecStrings, 
                                 int width /* = 0 */, char fill /* = '0' */) const
{
    vector<string>::reverse_iterator it = vecStrings.rbegin();
    if (width == 0)
    {
        width = it->length();
    }
    else
    {
        if (static_cast<size_t>(width) < it->length() || width < 0)
        {
            AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Range Width");
            return false;
        }
    }

    while (it != vecStrings.rend())
    {
        it->insert(0, width - it->length(), fill);
        it ++;
    }

    return true;
}

vector<string> StringPattern::InternalSplit(const string& str, const char& cLeft, const char& cRight) const
{
    if (!str.empty())
    {
        vector<string> strVector;
        string::size_type pos;
        string::size_type posBegin;
        string::size_type posMiddle;
        string::size_type posEnd;
        string s;
        if ((posBegin = str.find(cLeft)) != string::npos)
        {
            if (cLeft == '(' && (pos = str.find(',')) == string::npos)
            {
                AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Range Input");
            }

            while ((posMiddle = str.find(',', posBegin + 1)) != string::npos)
            {
                s = TrimString(str.substr(posBegin + 1, posMiddle - posBegin -1));
                if (!s.empty()) 
                {
                    strVector.push_back(s);
                } 
                else 
                {
                    AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Range Input");
                }
                posBegin = posMiddle;
            }
            if ((posEnd = str.rfind(cRight)) != string::npos)
            {
                if (posEnd < posBegin)
                {
                    std::cout << str <<endl;
                    AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format XXXX");
                }
                s = TrimString(str.substr(posBegin + 1, posEnd - posBegin -1));
                if (!s.empty()) 
                {
                    strVector.push_back(s);
                } 
                else 
                {
                    AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format : string excepted");
                }
                return strVector;
            } 
            else
            {
                AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format");
            }
        } 
        else
        {
            AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format");
        }
    }
    else
    {
        return std::vector<std::string>();
    }
}

string StringPattern::QuoteStrip(const string& str) const
{
    string::size_type posBegin = str.find('"');
    string::size_type posEnd = str.rfind('"');
    if (posBegin != string::npos && posEnd != string::npos && posBegin < posEnd)
    {
        return TrimString(str.substr(posBegin + 1, posEnd - posBegin -1));
    } else
    {
        AUTIL_LEGACY_THROW(ParameterInvalidException, "Invalid Input Format : \" excepted");
    }
}

}} // namespace autil::legacy

