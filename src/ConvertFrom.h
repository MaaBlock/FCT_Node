//
// Created by Administrator on 2025/5/29.
//

#ifndef CONVERTFROM_H
#define CONVERTFROM_H
namespace FCT
{
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

    template<>
    inline bool convertFromJS<bool>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsBoolean()) {
            return jsValue->BooleanValue(isolate);
        }
        return false;
    }

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

    template<>
    inline JSObject convertFromJS<JSObject>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsObject()) {
            v8::Local<v8::Object> jsObject = jsValue.As<v8::Object>();
            return JSObject(&env, isolate, jsObject);
        }
        return JSObject(&env, isolate, v8::Object::New(isolate));
    }
    template<>
    inline JSPromise convertFromJS<JSPromise>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();

        if (jsValue->IsPromise()) {
            v8::Local<v8::Promise> jsPromise = jsValue.As<v8::Promise>();
            return JSPromise(&env, isolate, jsPromise);
        }
        throw NodeError("Expected a Promise");
    }
    template<>
    inline JSArray convertFromJS<JSArray>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsArray()) {
            v8::Local<v8::Array> jsArray = jsValue.As<v8::Array>();
            return JSArray(&env, isolate, jsArray);
        }
        throw NodeError("Expected a Array");
    }
    template<>
    inline int convertFromJS<int>(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
        v8::Isolate* isolate = env.isolate();
        if (jsValue->IsNumber()) {
            return jsValue->Int32Value(env.context()).FromMaybe(0);
        }
        return 0;
    }

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
