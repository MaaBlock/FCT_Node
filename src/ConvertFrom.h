/**
* @brief Convert JavaScript function to C++ std::function
    * @tparam ReturnType Return type of the function
    * @tparam Args Argument types of the function
    * @param isolate V8 isolate instance
    * @param value JavaScript function value
    * @return C++ std::function wrapper, nullptr if not a function
    * @details Creates a callable C++ function that invokes the JavaScript function.
    *          Users can add their own template specializations for custom types by
    *          providing template<> specializations of convertFromJS<YourType>.
    * @code
    * auto jsFunc = convertFromJS<int, int, int>(isolate, jsFunctionValue);
    * int result = jsFunc(5, 3); // Calls JavaScript function with arguments
    *
    * // Custom type specialization example:
    * template<>
    * inline MyCustomType convertFromJS<MyCustomType>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
    *     // Your custom conversion logic here
    *     return MyCustomType();
    * }
    * @endcode
    */
#ifndef CONVERTFROM_H
#define CONVERTFROM_H
namespace FCT
{
    /**
   * @brief Generic template for converting JavaScript values to C++ types
   * @tparam T Target C++ type
   * @param env Node environment reference
   * @param jsValue JavaScript value to convert
   * @return Converted C++ value or default-constructed value if conversion fails
   * @details This is the fallback template that provides error reporting for unsupported types
   */
    template<typename T>
T convertFromJS(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        std::cerr << "Warning: No convertFromJS specialization for type '"
                  << typeid(T).name() << "'. Returning default-constructed value." << std::endl;

        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsUndefined()) {
            std::cerr << "  JavaScript value type: undefined" << std::endl;
        } else if (jsValue->IsNull()) {
            std::cerr << "  JavaScript value type: null" << std::endl;
        } else if (jsValue->IsBoolean()) {
            std::cerr << "  JavaScript value type: boolean" << std::endl;
        } else if (jsValue->IsNumber()) {
            std::cerr << "  JavaScript value type: number" << std::endl;
        } else if (jsValue->IsString()) {
            std::cerr << "  JavaScript value type: string" << std::endl;
        } else if (jsValue->IsArray()) {
            std::cerr << "  JavaScript value type: array" << std::endl;
        } else if (jsValue->IsObject()) {
            std::cerr << "  JavaScript value type: object" << std::endl;
        } else if (jsValue->IsFunction()) {
            std::cerr << "  JavaScript value type: function" << std::endl;
        } else {
            std::cerr << "  JavaScript value type: unknown" << std::endl;
        }

        return T();
    }
    /**
        * @brief Convert JavaScript boolean to C++ bool
        * @param env Node environment reference
        * @param jsValue JavaScript boolean value
        * @return C++ bool value, false if not a boolean
        */
    template<>
    inline bool convertFromJS<bool>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsBoolean()) {
            return jsValue->BooleanValue(isolate);
        }
        return false;
    }
    /**
       * @brief Convert JavaScript string to C++ std::string
       * @param env Node environment reference
       * @param jsValue JavaScript string value
       * @return C++ std::string, empty string if not a string
       */
    template<>
    inline std::string convertFromJS<std::string>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsString()) {
            v8::String::Utf8Value utf8Value(isolate, jsValue);
            if (*utf8Value) {
                return std::string(*utf8Value);
            }
        }
        return "";
    }
    /**
        * @brief Convert JavaScript string to C++ std::u8string
        * @param env Node environment reference
        * @param jsValue JavaScript string value
        * @return C++ std::u8string, empty string if not a string
        */
    template<>
    inline std::u8string convertFromJS<std::u8string>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsString()) {
            v8::String::Utf8Value utf8Value(isolate, jsValue);
            if (*utf8Value) {
                return std::u8string(reinterpret_cast<const char8_t*>(*utf8Value));
            }
        }
        return std::u8string();
    }
    /**
       * @brief Convert JavaScript object to JSObject wrapper
       * @param env Node environment reference
       * @param jsValue JavaScript object value
       * @return JSObject wrapper, empty object if not an object
    */
    template<>
    inline JSObject convertFromJS<JSObject>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsObject()) {
            v8::Local<v8::Object> jsObject = jsValue.As<v8::Object>();
            return JSObject(&env, isolate, jsObject);
        }
        return JSObject(&env, isolate, v8::Object::New(isolate));
    }
    /**
        * @brief Convert JavaScript value to C++ JSAny
        * @param env Node environment reference
        * @param jsValue JavaScript value of any type
        * @return C++ JSAny wrapping the JavaScript value
        */
    template<>
    inline JSAny convertFromJS<JSAny>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        return JSAny(&env, isolate, jsValue);
    }
    /**
     * @brief Convert JavaScript Promise to JSPromise wrapper
     * @param env Node environment reference
     * @param jsValue JavaScript Promise value
     * @return JSPromise wrapper
     * @throws NodeError if value is not a Promise
     */
    template<>
    inline JSPromise convertFromJS<JSPromise>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();

        if (jsValue->IsPromise()) {
            v8::Local<v8::Promise> jsPromise = jsValue.As<v8::Promise>();
            return JSPromise(&env, isolate, jsPromise);
        }
        throw NodeError("Expected a Promise");
    }
    /**
     * @brief Convert JavaScript Array to JSArray wrapper
     * @param env Node environment reference
     * @param jsValue JavaScript Array value
     * @return JSArray wrapper
     * @throws NodeError if value is not an Array
     */
    template<>
    inline JSArray convertFromJS<JSArray>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsArray()) {
            v8::Local<v8::Array> jsArray = jsValue.As<v8::Array>();
            return JSArray(&env, isolate, jsArray);
        }
        throw NodeError("Expected a Array");
    }
    /**
   * @brief Convert JavaScript number to C++ int
   * @param env Node environment reference
   * @param jsValue JavaScript number value
   * @return C++ int value, 0 if not a number
   */
    template<>
    inline int convertFromJS<int>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsNumber()) {
            return jsValue->Int32Value(env.context()).FromMaybe(0);
        }
        return 0;
    }
    /**
   * @brief Convert JavaScript number to C++ unsigned int
   * @param env Node environment reference
   * @param jsValue JavaScript number value
   * @return C++ unsigned int value, 0 if not a number
   */
    template<>
    inline unsigned int convertFromJS<unsigned int>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsNumber()) {
            return jsValue->Uint32Value(env.context()).FromMaybe(0);
        }
        return 0;
    }
    /**
   * @brief Convert JavaScript number to C++ unsigned __int64
   * @param env Node environment reference
   * @param jsValue JavaScript number value
   * @return C++ unsigned __int64 value, 0 if not a number
   */
    template<>
   inline uint64_t convertFromJS<uint64_t>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsNumber()) {
            return static_cast<uint64_t>(jsValue->IntegerValue(env.context()).FromMaybe(0));
        }
        return 0;
    }
    /**
   * @brief Convert JavaScript number to C++ float
   * @param env Node environment reference
   * @param jsValue JavaScript number value
   * @return C++ float value, 0.0f if not a number
   */
    template<>
    inline float convertFromJS<float>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsNumber()) {
            return static_cast<float>(jsValue->NumberValue(env.context()).FromMaybe(0.0));
        }
        return 0.0f;
    }
    /**
   * @brief Convert JavaScript number to C++ double
   * @param env Node environment reference
   * @param jsValue JavaScript number value
   * @return C++ double value, 0.0 if not a number
   */
    template<>
    inline double convertFromJS<double>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsNumber()) {
            return jsValue->NumberValue(env.context()).FromMaybe(0.0);
        }
        return 0.0;
    }
    /**
        * @brief Convert JavaScript function to C++ std::function
        * @tparam ReturnType Return type of the function
        * @tparam Args Argument types of the function
        * @param isolate V8 isolate instance
        * @param value JavaScript function value
        * @return C++ std::function wrapper, nullptr if not a function
        * @details Creates a callable C++ function that invokes the JavaScript function
        * @example
        * @code
        * auto jsFunc = convertFromJS<int, int, int>(isolate, jsFunctionValue);
        * int result = jsFunc(5, 3); // Calls JavaScript function with arguments
        * @endcode
        */
    template<typename ReturnType, typename... Args>
   inline std::function<ReturnType(Args...)> convertFromJS(v8::Isolate* isolate,
                                                          v8::Local<v8::Value> value) {
        if (!value->IsFunction()) {
            return nullptr;
        }

        v8::Local<v8::Function> jsFunc = value.As<v8::Function>();
        v8::Global<v8::Function> persistentFunc(isolate, jsFunc);

        return [isolate, persistentFunc = std::move(persistentFunc)](Args... args) -> ReturnType {
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Context> context = isolate->GetCurrentContext();
            v8::Context::Scope context_scope(context);

            v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, persistentFunc);

            std::vector<v8::Local<v8::Value>> jsArgs;
            if constexpr (sizeof...(args) > 0) {
                jsArgs.reserve(sizeof...(args));
                (jsArgs.push_back(convertToJS(isolate, args)), ...);
            }

            v8::TryCatch try_catch(isolate);
            v8::MaybeLocal<v8::Value> result = func->Call(context, context->Global(),
                                                          jsArgs.size(), jsArgs.data());

            if (try_catch.HasCaught()) {
                v8::Local<v8::Value> exception = try_catch.Exception();
                v8::String::Utf8Value exception_str(isolate, exception);
                throw std::runtime_error("JavaScript function error: " + std::string(*exception_str));
            }

            if constexpr (!std::is_void_v<ReturnType>) {
                if (result.IsEmpty()) {
                    return ReturnType{};
                }
                return convertFromJS<ReturnType>(isolate, result.ToLocalChecked());
            }
        };
    }
}
#endif //CONVERTFROM_H
