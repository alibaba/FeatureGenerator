#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <typeinfo>
#include <utility>

#include "autil/legacy/exception.h"

namespace autil{ namespace legacy
{
class Any;

namespace json {
// json_string_enhanced.cpp
void ToString(const Any& a, std::string& ret, bool isCompact = false, const std::string& prefix = "");

}

    class Any {
        template<typename ValueType>
        friend const ValueType& AnyCast(const Any& operand);

        template<typename ValueType>
        friend const ValueType* AnyCast(const Any* operand);

        template<typename ValueType>
        friend ValueType* AnyCast(Any* operand);

    public:
        Any()
        {}

        template<typename ValueType> Any(const ValueType& value)
            : mContent(new Wrapper<ValueType>(value))
        {}

        template<typename ValueType> Any & operator=(const ValueType& value)
        {
            Any(value).Swap(*this);
            return *this;
        }

        void Swap(Any& other)
        {
            std::swap(mContent, other.mContent);
        }

        bool IsEmpty() const
        {
            return !mContent;
        }

        const std::type_info& GetType() const
        {
            if (mContent)
                return mContent->GetType();
            else
                return typeid(void);
        }

    private:
        struct WrapperBase
        {
        public:
            virtual ~WrapperBase() {}
            virtual const std::type_info& GetType() const =0;
        };

        template<typename ValueType>
        struct Wrapper : public WrapperBase
        {
            Wrapper(const ValueType& value) : mValue(value) {}

            /* override */ const std::type_info& GetType() const
            {
                return typeid(ValueType);
            }

            ValueType mValue;
        };

        std::shared_ptr<WrapperBase> mContent;
    };

    class BadAnyCast : public ExceptionBase
    {
    public:
        AUTIL_LEGACY_DEFINE_EXCEPTION(BadAnyCast, ExceptionBase)
    };

    class BadAnyNumberCast : public BadAnyCast
    {
    public:
        AUTIL_LEGACY_DEFINE_EXCEPTION(BadAnyNumberCast, BadAnyCast)

        /* override */ std::string GetMessage() const
        {
            return "autil::legacy::AnyNumberCast: Given is not of number type.";
        }
    };

    template<typename ValueType>
    const ValueType& AnyCast(const Any& operand)
    {
        if (operand.GetType() == typeid(ValueType)) {
            return static_cast<const Any::Wrapper<ValueType>*>(operand.mContent.get())->mValue;
        } else {
            std::string operandStr;
            json::ToString(operand, operandStr, true, "");
            AUTIL_LEGACY_THROW(BadAnyCast,
                    std::string("autil::legacy::AnyCast: can't cast from ")
                    + operand.GetType().name() + " to " + typeid(ValueType).name()
                    + ", content is " + operandStr);
        }
    }

    template<typename ValueType>
    const ValueType* AnyCast(const Any* operand)
    {
        if (operand && operand->GetType()==typeid(ValueType))
            return &static_cast<const Any::Wrapper<ValueType>*>(operand->mContent.get())->mValue;
        else
            return NULL;
    }

    template<typename ValueType>
    ValueType* AnyCast(Any* operand)
    {
        if (operand && operand->GetType()==typeid(ValueType))
            return &static_cast<Any::Wrapper<ValueType>*>(operand->mContent.get())->mValue;
        else
            return NULL;
    }

    /**
     * Cast a known number-type Any to a specified number type.
     *
     * When already know one Any is a number but not sure what the exact
     * type it is, one has to check the real type of the Any and cast it
     * accordingly. This method plans to wrap the check-then-cast logic.
     *
     * @param number An Any instance, expecting to be a number type. Exception
     * will be thrown if 'number' is not of a number type.
     * @return cast the Any to the expected type T.
     *
     * @author XING Dikan (dikan.xingdk@alibaba-inc.com)
     */
    template<typename T>
    T AnyNumberCast(const Any& number)
    {
        const std::type_info& type = number.GetType();
#define IF_CAST_RETURN(tp) \
        if (type == typeid(tp)) \
        { \
            return static_cast<T>(AnyCast<tp>(number)); \
        }
        IF_CAST_RETURN(uint8_t)
        else
        IF_CAST_RETURN(int8_t)
        else
        IF_CAST_RETURN(uint16_t)
        else
        IF_CAST_RETURN(int16_t)
        else
        IF_CAST_RETURN(uint32_t)
        else
        IF_CAST_RETURN(int32_t)
        else
        IF_CAST_RETURN(uint64_t)
        else
        IF_CAST_RETURN(int64_t)
        else
        IF_CAST_RETURN(float)
        else
        IF_CAST_RETURN(double)
        else
        {
            AUTIL_LEGACY_THROW(BadAnyNumberCast, "");
        }
#undef IF_CAST_RETURN
    }
}}

