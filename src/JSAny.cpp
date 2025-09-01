//
// Created by Administrator on 2025/5/27.
//

#include "headers.h"

namespace FCT {
    JSAny::JSAny(NodeEnvironment* env, v8::Isolate* isolate, v8::Global<v8::Value> value): m_isolate(isolate), m_value(std::move(value))
    {
        if (isolate && m_value.IsEmpty()) {
            v8::HandleScope scope(isolate);
            v8::Local<v8::Value> undefinedValue = v8::Undefined(isolate);
            m_value.Reset(isolate, undefinedValue);
        }
        m_env = env;
    }
    
    JSAny::JSAny(NodeEnvironment* env, v8::Isolate* isolate, v8::Local<v8::Value> value): m_isolate(isolate)
    {
        m_value.Reset(isolate, value);
        m_env = env;
    }

    JSAny::JSAny(JSAny&& other) noexcept: m_isolate(other.m_isolate), m_env(other.m_env)
    {
        if (!other.m_value.IsEmpty()) {
            v8::HandleScope handleScope(other.m_isolate);
            v8::Local<v8::Value> localValue = other.m_value.Get(other.m_isolate);
            m_value.Reset(other.m_isolate, localValue);
            other.m_value.Reset();
        }
        other.m_env = nullptr;
    }
} // FCT