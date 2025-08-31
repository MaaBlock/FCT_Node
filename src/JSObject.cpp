//
// Created by Administrator on 2025/5/27.
//

#include "headers.h"

namespace FCT {
    JSObject::JSObject(NodeEnvironment* env,v8::Isolate* isolate, v8::Global<v8::Object> object): m_isolate(isolate), m_object(std::move(object))
    {
        if (isolate && m_object.IsEmpty()) {
            v8::HandleScope scope(isolate);
            v8::Local<v8::Object> emptyObj = v8::Object::New(isolate);
            m_object.Reset(isolate, emptyObj);
        }
        m_env = env;
    }
    JSObject::JSObject(NodeEnvironment* env,v8::Isolate* isolate, v8::Local<v8::Object> object): m_isolate(isolate)
    {
        m_object.Reset(isolate, object);
        m_env = env;
    }

    JSObject::JSObject(JSObject&& other) noexcept: m_isolate(other.m_isolate)
    {
        if (!other.m_object.IsEmpty()) {
            v8::HandleScope handleScope(other.m_isolate);
            v8::Local<v8::Object> localObj = other.m_object.Get(other.m_isolate);
            m_object.Reset(other.m_isolate, localObj);
            other.m_object.Reset();
        }
    }

    bool JSObject::hasProperty(const std::string& propertyName) const
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_env->context();
        v8::Context::Scope context_scope(context);
        v8::Local<v8::Object> obj = getLocalObject();

        v8::Local<v8::String> key = v8::String::NewFromUtf8(m_isolate, propertyName.c_str()).ToLocalChecked();
        return obj->Has(context, key).FromMaybe(false);
    }

    std::vector<std::string> JSObject::getPropertyNames() const
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_env->context();
        if (context.IsEmpty()) {
            std::cout << "empty context" << std::endl;
            return {};
        }

        v8::Context::Scope context_scope(context);
        v8::Local<v8::Object> obj = getLocalObject();
        if (obj.IsEmpty())
        {
            std::cout << "empty obj" << std::endl;
        }
        v8::MaybeLocal<v8::Array> maybeProps = obj->GetOwnPropertyNames(context);
        if (maybeProps.IsEmpty()) {
            return {};
        }

        v8::Local<v8::Array> props = maybeProps.ToLocalChecked();
        std::vector<std::string> result;
        uint32_t length = props->Length();

        for (uint32_t i = 0; i < length; i++) {
            v8::MaybeLocal<v8::Value> maybeProp = props->Get(context, i);
            if (maybeProp.IsEmpty()) continue;

            v8::Local<v8::Value> prop = maybeProp.ToLocalChecked();
            v8::String::Utf8Value utf8Value(m_isolate, prop);
            if (*utf8Value) {
                result.push_back(std::string(*utf8Value));
            }
        }

        return result;
    }

    std::vector<std::string> JSObject::getFunctionNames() const
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_env->context();
        if (context.IsEmpty()) {
            std::cout << "empty context" << std::endl;
            return {};
        }

        v8::Context::Scope context_scope(context);
        v8::Local<v8::Object> obj = getLocalObject();
        if (obj.IsEmpty())
        {
            std::cout << "empty obj" << std::endl;
            return {};
        }

        v8::MaybeLocal<v8::Array> maybeProps = obj->GetOwnPropertyNames(context);
        if (maybeProps.IsEmpty()) {
            return {};
        }

        v8::Local<v8::Array> props = maybeProps.ToLocalChecked();
        std::vector<std::string> result;
        uint32_t length = props->Length();

        for (uint32_t i = 0; i < length; i++) {
            v8::MaybeLocal<v8::Value> maybeProp = props->Get(context, i);
            if (maybeProp.IsEmpty()) continue;

            v8::Local<v8::Value> prop = maybeProp.ToLocalChecked();
            v8::String::Utf8Value utf8Value(m_isolate, prop);
            if (*utf8Value) {
                std::string propName(*utf8Value);

                v8::Local<v8::String> key = v8::String::NewFromUtf8(m_isolate, propName.c_str()).ToLocalChecked();
                v8::MaybeLocal<v8::Value> maybeValue = obj->Get(context, key);
                if (!maybeValue.IsEmpty()) {
                    v8::Local<v8::Value> value = maybeValue.ToLocalChecked();
                    if (value->IsFunction()) {
                        result.push_back(propName);
                    }
                }
            }
        }

        return result;
    }
} // FCT