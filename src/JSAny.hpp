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
            return T{};
        }
        v8::Locker locker(m_isolate);
        v8::HandleScope scope(m_isolate);
        return convertFromJS<T>(*m_env, getValue());
    }
    
    template<typename T>
    T JSAny::to() const {
        if (m_value.IsEmpty()) {
            throw std::runtime_error("Cannot convert empty JSAny to target type");
        }
        v8::Locker locker(m_isolate);
        v8::HandleScope scope(m_isolate);
        return convertFromJS<T>(*m_env, getValue());
    }
    
    // convertToJS specializations for JSAny
    template<>
    inline v8::Local<v8::Value> convertToJS<JSAny>(NodeEnvironment& env, const JSAny& arg) {
        return arg.value();
    }
    
} // namespace FCT

#endif // JSANY_HPP