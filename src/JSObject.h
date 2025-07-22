JSObject.h
/**
 * @file JSObject.h
 * @brief JavaScript Object wrapper for C++ interoperability
 * @details This file provides a C++ wrapper class for JavaScript objects,
 *          enabling seamless manipulation of JavaScript objects from C++ code.
 *          Supports type-safe property access, modification, and object introspection.
 * @author FCT Team
 */

#ifndef JSOBJECT_H
#define JSOBJECT_H
namespace FCT {
    class NodeEnvironment;
    /**
     * @brief C++ wrapper for JavaScript Object instances
     * @details Provides a convenient interface for manipulating JavaScript objects
     *          from C++ code with automatic type conversion and memory management.
     *          The class manages V8 handles and provides RAII semantics.
     */
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
        /**
          * @brief Get local handle to the JavaScript object
          * @return Local handle to the object
          */
        v8::Local<v8::Object> getLocalObject() const {
            return m_object.Get(m_isolate);
        }
        /**
          * @brief Check if the object handle is null/empty
          * @return true if object is null or empty, false otherwise
          */
        bool isNull() const
        {
            return m_object.IsEmpty();
        }
        /**
          * @brief Get property value with type conversion
          * @tparam T Target C++ type
          * @param propertyName Name of the property to get
          * @return Converted value of type T
          * @details Automatically converts JavaScript property value to requested C++ type
          */
        template<typename T>
        T get(const std::string& propertyName) const;
        /**
          * @brief Set property value with type conversion
          * @tparam T Source C++ type
          * @param propertyName Name of the property to set
          * @param value Value to set
          * @return true if successful, false otherwise
          * @details Automatically converts C++ value to JavaScript value
          */
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
        /**
          * @brief Property access operator
          * @param propertyName Name of the property to access
          * @return PropertyProxy for the specified property
          * @details Enables natural object syntax: obj["prop"] = value; int x = obj["prop"];
          */
        PropertyProxy operator[](const std::string& propertyName) const {
            return PropertyProxy(*this, propertyName);
        }
        /**
          * @brief Check if object has a specific property
          * @param propertyName Name of the property to check
          * @return true if property exists, false otherwise
          */
        bool hasProperty(const std::string& propertyName) const;
        /**
          * @brief Get all property names of the object
          * @return Vector of property names as strings
          * @details Returns enumerable own properties of the object
          */
        std::vector<std::string> getPropertyNames() const;
    };
} // FCT

#endif //JSOBJECT_H
