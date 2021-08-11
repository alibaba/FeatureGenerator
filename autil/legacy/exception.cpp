#include "autil/legacy/exception.h"
#include <execinfo.h>
#include <stdlib.h>
#include <cxxabi.h>

namespace autil{ namespace legacy
{

ExceptionBase::ExceptionBase(const std::string& message) throw()
    : mMessage(message),
      mFile("<unknown file>"),
      mFunction("<unknown function>"),
      mLine(-1),
      mStackTraceSize(0)
{}

ExceptionBase::~ExceptionBase() throw()
{}

std::shared_ptr<ExceptionBase> ExceptionBase::Clone() const
{
    return std::shared_ptr<ExceptionBase>(new ExceptionBase(*this));
}

void ExceptionBase::Init(const char* file, const char* function, int line)
{
    mFile = file;
    mFunction = function;
    mLine = line;
    mStackTraceSize = backtrace(mStackTrace, MAX_STACK_TRACE_SIZE);
}

void ExceptionBase::SetCause(const ExceptionBase& cause)
{
    SetCause(cause.Clone());
}

void ExceptionBase::SetCause(std::shared_ptr<ExceptionBase> cause)
{
    mNestedException = cause;
}

std::shared_ptr<ExceptionBase> ExceptionBase::GetCause() const
{
    return mNestedException;
}

std::shared_ptr<ExceptionBase> ExceptionBase::GetRootCause() const
{
    if (mNestedException.get())
    {
        std::shared_ptr<ExceptionBase> rootCause =  mNestedException->GetRootCause();
        if (rootCause.get())
            return rootCause;
    }
    return mNestedException;
}

std::string ExceptionBase::GetClassName() const
{
    return "ExceptionBase";
}

std::string ExceptionBase::GetMessage() const
{
    return mMessage;
}

const char* ExceptionBase::what() const throw()
{
    return ToString().c_str();
}

const std::string& ExceptionBase::ToString() const
{
    if (mWhat.empty())
    {
        if (mLine > 0)
            mWhat = std::string(mFile) + "(" + std::to_string(mLine) + ")";
        else
            mWhat = "<unknown throw location>";
        mWhat += ": " + GetClassName();
        std::string customizedString = GetMessage();
        if (!customizedString.empty())
        {
            mWhat += ": " + customizedString;
        }
        mWhat += "\nStack trace:\n";
        mWhat += GetStackTrace();
        if (mNestedException.get())
        {
            mWhat += "Caused by:\n" + mNestedException->ToString();
        }
    }
    return mWhat;
}

const std::string& ExceptionBase::GetExceptionChain() const
{
    return ToString();
}

std::string ExceptionBase::GetStackTrace() const
{
    if (mStackTraceSize == 0)
        return "<No stack trace>\n";
    char** strings = backtrace_symbols(mStackTrace, mStackTraceSize);
    if (strings == NULL) // Since this is for debug only thus
                         // non-critical, don't throw an exception.
        return "<Unknown error: backtrace_symbols returned NULL>\n";

    std::string result;
    for (size_t i = 0; i < mStackTraceSize; ++i)
    {
        std::string mangledName = strings[i];
        std::string::size_type begin = mangledName.find('(');
        std::string::size_type end = mangledName.find('+', begin);
        if (begin == std::string::npos || end == std::string::npos)
        {
            result += mangledName;
            result += '\n';
            continue;
        }
        ++begin;
        int status;
        char* s = abi::__cxa_demangle(mangledName.substr(begin, end-begin).c_str(),
                                      NULL, 0, &status);
        if (status != 0)
        {
            result += mangledName;
            result += '\n';
            continue;
        }
        std::string demangledName(s);
        free(s);
        // Ignore ExceptionBase::Init so the top frame is the
        // user's frame where this exception is thrown.
        //
        // Can't just ignore frame#0 because the compiler might
        // inline ExceptionBase::Init.
        if (i == 0
            && demangledName == "autil::legacy::ExceptionBase::Init(char const*, char const*, int)")
            continue;
        result += mangledName.substr(0, begin);
        result += demangledName;
        result += mangledName.substr(end);
        result += '\n';
    }
    free(strings);
    return result;
}

}}//namespace autil::legacy
