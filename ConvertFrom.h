//
// Created by Administrator on 2025/5/29.
//

#ifndef CONVERTFROM_H
#define CONVERTFROM_H
namespace FCT
{
    template<typename T>
    T convertFromJS(NodeEnvironment& env, v8::Local<v8::Value> jsValue) {
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
}
#endif //CONVERTFROM_H
