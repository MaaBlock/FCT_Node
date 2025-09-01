/**
 * @file JSAny.h
 * @brief JavaScript Any value wrapper for C++ interoperability
 * @details This file provides a C++ wrapper class for any JavaScript value,
 *          enabling seamless manipulation of JavaScript values from C++ code.
 *          Supports type-safe value access, modification, and type checking.
 * @author FCT Team
 */

#ifndef JSANY_H
#define JSANY_H
namespace FCT {
    class NodeEnvironment;
    /**
     * @brief C++ wrapper for any JavaScript value
     * @details Provides a convenient interface for manipulating JavaScript values
     *          from C++ code with automatic type conversion and memory management.
     *          The class manages V8 handles and provides RAII semantics.
     */
    class JSAny {
    private:
        v8::Isolate* m_isolate;
        v8::Global<v8::Value> m_value;
        NodeEnvironment* m_env;
    public:
        JSAny(NodeEnvironment* env, v8::Isolate* isolate, v8::Global<v8::Value> value);
        JSAny(NodeEnvironment* env, v8::Isolate* isolate, v8::Local<v8::Value> value);

        ~JSAny() {
            m_value.Reset();
        }

        JSAny(const JSAny&) = delete;
        JSAny& operator=(const JSAny&) = delete;

        JSAny(JSAny&& other) noexcept;

        JSAny& operator=(JSAny&& other) noexcept {
            if (this != &other) {
                m_value.Reset();
                m_isolate = other.m_isolate;
                m_env = other.m_env;
                m_value.Reset(other.m_isolate, other.getValue());
                other.m_value.Reset();
                other.m_env = nullptr;
            }
            return *this;
        }

        /**
          * @brief Get the local V8 value handle
          * @return Local handle to the value
          */
        v8::Local<v8::Value> getValue() const {
            return m_value.Get(m_isolate);
        }
        
        /**
          * @brief Check if the value handle is null/empty
          * @return true if value is null or empty, false otherwise
          */
        bool isNull() const {
            return m_value.IsEmpty();
        }
        
        /**
          * @brief Check if the value is undefined
          * @return true if value is undefined, false otherwise
          */
        bool isUndefined() const {
            if (m_value.IsEmpty()) return true;
            return getValue()->IsUndefined();
        }
        
        /**
          * @brief Check if the value is a boolean
          * @return true if value is boolean, false otherwise
          */
        bool isBoolean() const {
            if (m_value.IsEmpty()) return false;
            return getValue()->IsBoolean();
        }
        
        /**
          * @brief Check if the value is a number
          * @return true if value is number, false otherwise
          */
        bool isNumber() const {
            if (m_value.IsEmpty()) return false;
            return getValue()->IsNumber();
        }
        
        /**
          * @brief Check if the value is a string
          * @return true if value is string, false otherwise
          */
        bool isString() const {
            if (m_value.IsEmpty()) return false;
            return getValue()->IsString();
        }
        
        /**
          * @brief Check if the value is an object
          * @return true if value is object, false otherwise
          */
        bool isObject() const {
            if (m_value.IsEmpty()) return false;
            return getValue()->IsObject();
        }
        
        /**
          * @brief Check if the value is an array
          * @return true if value is array, false otherwise
          */
        bool isArray() const {
            if (m_value.IsEmpty()) return false;
            return getValue()->IsArray();
        }
        
        /**
          * @brief Check if the value is a function
          * @return true if value is function, false otherwise
          */
        bool isFunction() const {
            if (m_value.IsEmpty()) return false;
            return getValue()->IsFunction();
        }
        
        /**
          * @brief Convert to C++ type with automatic type conversion
          * @tparam T Target C++ type
          * @return Converted value
          */
        template<typename T>
        T as() const;
        
        /**
          * @brief Convert JavaScript value to C++ type using convertFromJS
          * @tparam T Target C++ type
          * @return Converted value
          * @details Uses the convertFromJS template function to convert the underlying
          *          JavaScript value to the specified C++ type
          */
        template<typename T>
        T to() const;
        
        /**
          * @brief Get the JavaScript value representation
          * @return v8::Local<v8::Value> representing this value
          * @details Returns the underlying JavaScript value
          */
        v8::Local<v8::Value> value() const {
            return getValue();
        }
    };
} // FCT

#endif //JSANY_H