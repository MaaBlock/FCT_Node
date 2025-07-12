//
// Created by Administrator on 2025/5/27.
//

#ifndef JSOBJECT_H
#define JSOBJECT_H
#include "NodeEnvironment.h"
#include "ConvertTo.h"
namespace FCT {
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
        bool set(const std::string& propertyName, const T& value);

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

        bool hasProperty(const std::string& propertyName) const;

        std::vector<std::string> getPropertyNames() const;
    };
    template<>
    inline v8::Local<v8::Value> convertToJS<JSObject>(v8::Isolate* isolate, JSObject arg) {
        return arg.getLocalObject();
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const JSObject&>(v8::Isolate* isolate, const JSObject& arg) {
        return arg.getLocalObject();
    }
} // FCT

#endif //JSOBJECT_H
