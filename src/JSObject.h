//
// Created by Administrator on 2025/5/27.
//

#ifndef JSOBJECT_H
#define JSOBJECT_H
#include "NodeEnvironment.h"

namespace FCT {
    template<typename T>
    v8::Local<v8::Value> convertToJS(v8::Isolate* isolate,T arg)
    {
        return v8::Undefined(isolate);
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<const std::string&>(v8::Isolate* isolate,const std::string& arg)
    {
        return v8::String::NewFromUtf8(isolate, arg.c_str()).ToLocalChecked();
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<std::string&&>(v8::Isolate* isolate,std::string&& arg)
    {
        return v8::String::NewFromUtf8(isolate, arg.c_str()).ToLocalChecked();
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<std::string>(v8::Isolate* isolate, std::string arg)
    {
        return v8::String::NewFromUtf8(isolate, arg.c_str()).ToLocalChecked();
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<const char*>(v8::Isolate* isolate, const char* arg) {
        return v8::String::NewFromUtf8(isolate, arg).ToLocalChecked();
    }
    class NodeEnvironment;
    class JSObject {
    private:
        v8::Isolate* m_isolate;
        v8::Global<v8::Object> m_object;
        NodeEnvironment* m_env;
    public:
        JSObject(NodeEnvironment* env,v8::Isolate* isolate, v8::Global<v8::Object> object);
        JSObject(NodeEnvironment* env,v8::Isolate* isolate, v8::Local<v8::Object> object);

        ~JSObject() {
            m_object.Reset();
        }

        JSObject(const JSObject&) = delete;
        JSObject& operator=(const JSObject&) = delete;

        JSObject(JSObject&& other) noexcept;

        JSObject& operator=(JSObject&& other) noexcept {
            if (this != &other) {
                m_object.Reset();
                m_isolate = other.m_isolate;
                m_object.Reset(other.m_isolate, other.getLocalObject());
                other.m_object.Reset();
                std::cout << "right moved" << std::endl;
            }
            std::cout << "return this"<< std::endl;
            return *this;
        }

        v8::Local<v8::Object> getLocalObject() const {
            return m_object.Get(m_isolate);
        }

        bool isNull() const
        {
            return m_object.IsEmpty();
        }

        template<typename T>
        T get(const std::string& propertyName) const;

        template<typename T>
        bool set(const std::string& propertyName, const T& value) {
            v8::Locker locker(m_isolate);
            v8::HandleScope handleScope(m_isolate);
            v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
            v8::Local<v8::Object> obj = getLocalObject();

            v8::Local<v8::String> key = v8::String::NewFromUtf8(m_isolate, propertyName.c_str()).ToLocalChecked();
            v8::Local<v8::Value> jsValue = convertToJS(m_isolate, value);

            return obj->Set(context, key, jsValue).FromMaybe(false);
        }

        class PropertyProxy {
        private:
            const JSObject& m_jsObject;
            std::string m_propertyName;

        public:
            PropertyProxy(const JSObject& jsObject, const std::string& propertyName)
                : m_jsObject(jsObject), m_propertyName(propertyName) {}

            template<typename T>
            operator T() const {
                return m_jsObject.get<T>(m_propertyName);
            }

            template<typename T>
            PropertyProxy& operator=(const T& value) {
                const_cast<JSObject&>(m_jsObject).set(m_propertyName, value);
                return *this;
            }
        };

        PropertyProxy operator[](const std::string& propertyName) const {
            return PropertyProxy(*this, propertyName);
        }

        bool hasProperty(const std::string& propertyName) const {
            v8::HandleScope handleScope(m_isolate);
            v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
            v8::Local<v8::Object> obj = getLocalObject();

            v8::Local<v8::String> key = v8::String::NewFromUtf8(m_isolate, propertyName.c_str()).ToLocalChecked();
            return obj->Has(context, key).FromMaybe(false);
        }

        std::vector<std::string> getPropertyNames() const;
    };
} // FCT

#endif //JSOBJECT_H
