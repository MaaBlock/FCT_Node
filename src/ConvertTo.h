//
// Created by Administrator on 2025/7/12.
//

#ifndef CONVERTTO_H
#define CONVERTTO_H
namespace FCT
{
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

      template<>
    inline v8::Local<v8::Value> convertToJS<int>(v8::Isolate* isolate, int arg) {
        return v8::Integer::New(isolate, arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const int&>(v8::Isolate* isolate, const int& arg) {
        return v8::Integer::New(isolate, arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<unsigned int>(v8::Isolate* isolate, unsigned int arg) {
        return v8::Integer::NewFromUnsigned(isolate, arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<long>(v8::Isolate* isolate, long arg) {
        return v8::Number::New(isolate, static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<long long>(v8::Isolate* isolate, long long arg) {
        return v8::Number::New(isolate, static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<unsigned long>(v8::Isolate* isolate, unsigned long arg) {
        return v8::Number::New(isolate, static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<unsigned long long>(v8::Isolate* isolate, unsigned long long arg) {
        return v8::Number::New(isolate, static_cast<double>(arg));
    }

    // 浮点数类型特化
    template<>
    inline v8::Local<v8::Value> convertToJS<float>(v8::Isolate* isolate, float arg) {
        return v8::Number::New(isolate, static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<double>(v8::Isolate* isolate, double arg) {
        return v8::Number::New(isolate, arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const double&>(v8::Isolate* isolate, const double& arg) {
        return v8::Number::New(isolate, arg);
    }

    // 布尔类型特化
    template<>
    inline v8::Local<v8::Value> convertToJS<bool>(v8::Isolate* isolate, bool arg) {
        return v8::Boolean::New(isolate, arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const bool&>(v8::Isolate* isolate, const bool& arg) {
        return v8::Boolean::New(isolate, arg);
    }


}
#endif //CONVERTTO_H
