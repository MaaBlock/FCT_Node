/**
 * @file JSAny.hpp
 * @brief Template implementations for JSAny class
 * @details This file contains template method implementations and type conversion
 *          specializations for the JSAny class.
 * @author FCT Team
 */

#ifndef JSANY_HPP
#define JSANY_HPP

#include "JSAny.h"
#include "ConvertFrom.h"
#include "ConvertTo.h"
#include <stdexcept>

namespace FCT {
    
    template<typename T>
    T JSAny::as() const {
        if (m_value.IsEmpty()) {
            return T{}; // 返回默认值
        }
        return convertFromJS<T>(*m_env, getValue());
    }
    
    template<typename T>
    T JSAny::to() const {
        if (m_value.IsEmpty()) {
            // 对于没有默认构造函数的类型，直接抛出异常
            throw std::runtime_error("Cannot convert empty JSAny to target type");
        }
        return convertFromJS<T>(*m_env, getValue());
    }
    
    // convertToJS specializations for JSAny
    template<>
    inline v8::Local<v8::Value> convertToJS<JSAny>(NodeEnvironment& env, const JSAny& arg) {
        return arg.value();
    }
    
} // FCT

#endif //JSANY_HPP