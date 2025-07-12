//
// Created by Administrator on 2025/7/12.
//

#ifndef CONVERTTO_H
#define CONVERTTO_H
#include "NodeEnvironment.h"
#include "FunctionWrapper.h"
namespace FCT
{
    template<typename T>
    struct lambda_detector {
    private:
        using type = std::decay_t<T>;

        static constexpr bool is_class = std::is_class_v<type>;

        template<typename U>
        static auto has_call_operator(int) -> decltype(&U::operator(), std::true_type{});
        template<typename>
        static std::false_type has_call_operator(...);
        static constexpr bool has_call_op = decltype(has_call_operator<type>(0))::value;

        static constexpr bool is_not_string =
            !std::is_convertible_v<type, std::string> &&
            !std::is_convertible_v<type, const char*>;

        static constexpr bool is_not_std_function =
            !std::is_same_v<type, std::function<typename std::function<void()>::result_type()>>;

        template<typename U>
        static auto can_convert_to_function(int) ->
            decltype(std::function(std::declval<U>()), std::true_type{});
        template<typename>
        static std::false_type can_convert_to_function(...);
        static constexpr bool convertible_to_function =
            decltype(can_convert_to_function<type>(0))::value;

    public:
        static constexpr bool value =
            is_class &&
            has_call_op &&
            is_not_string &&
            is_not_std_function &&
            convertible_to_function;
    };

    template<typename T>
    constexpr bool is_lambda_v = lambda_detector<T>::value;

    template<typename Lambda>
    inline std::enable_if_t<is_lambda_v<Lambda>, v8::Local<v8::Value>>
    convertToJS(NodeEnvironment& env, Lambda&& lambda) {
        std::function func(std::forward<Lambda>(lambda));
        return convertToJS(env, std::move(func));
    }
    template<typename T>
    std::enable_if_t<!is_lambda_v<std::decay_t<T>>, v8::Local<v8::Value>>
    convertToJS(NodeEnvironment& env, T arg)
    {
        return v8::Undefined(env.isolate());
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<const std::string&>(NodeEnvironment& env, const std::string& arg)
    {
        return v8::String::NewFromUtf8(env.isolate(), arg.c_str()).ToLocalChecked();
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<std::string&&>(NodeEnvironment& env, std::string&& arg)
    {
        return v8::String::NewFromUtf8(env.isolate(), arg.c_str()).ToLocalChecked();
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<std::string>(NodeEnvironment& env, std::string arg)
    {
        return v8::String::NewFromUtf8(env.isolate(), arg.c_str()).ToLocalChecked();
    }
    template<>
    inline v8::Local<v8::Value> convertToJS<const char*>(NodeEnvironment& env, const char* arg) {
        return v8::String::NewFromUtf8(env.isolate(), arg).ToLocalChecked();
    }

    template<>
  inline v8::Local<v8::Value> convertToJS<int>(NodeEnvironment& env, int arg) {
        return v8::Integer::New(env.isolate(), arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const int&>(NodeEnvironment& env, const int& arg) {
        return v8::Integer::New(env.isolate(), arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<unsigned int>(NodeEnvironment& env, unsigned int arg) {
        return v8::Integer::NewFromUnsigned(env.isolate(), arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<long>(NodeEnvironment& env, long arg) {
        return v8::Number::New(env.isolate(), static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<long long>(NodeEnvironment& env, long long arg) {
        return v8::Number::New(env.isolate(), static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<unsigned long>(NodeEnvironment& env, unsigned long arg) {
        return v8::Number::New(env.isolate(), static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<unsigned long long>(NodeEnvironment& env, unsigned long long arg) {
        return v8::Number::New(env.isolate(), static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<float>(NodeEnvironment& env, float arg) {
        return v8::Number::New(env.isolate(), static_cast<double>(arg));
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<double>(NodeEnvironment& env, double arg) {
        return v8::Number::New(env.isolate(), arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const double&>(NodeEnvironment& env, const double& arg) {
        return v8::Number::New(env.isolate(), arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<bool>(NodeEnvironment& env, bool arg) {
        return v8::Boolean::New(env.isolate(), arg);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<const bool&>(NodeEnvironment& env, const bool& arg) {
        return v8::Boolean::New(env.isolate(), arg);
    }

    template<typename ReturnType, typename... Args>
    inline v8::Local<v8::Value> convertToJS(NodeEnvironment& env,
                                       std::function<ReturnType(Args...)> func) {
        if (!func) {
            return v8::Null(env.isolate());
        }

        auto wrapper = std::make_unique<FunctionWrapper<ReturnType, Args...>>(std::move(func), env);
        int64_t id = env.functionManager().registerFunction(std::move(wrapper));

        v8::Local<v8::Array> callbackData = v8::Array::New(env.isolate(), 2);
        callbackData->Set(env.context(), 0, v8::BigInt::New(env.isolate(), id)).Check();
        callbackData->Set(env.context(), 1, v8::External::New(env.isolate(), &env)).Check();

        v8::Local<v8::FunctionTemplate> funcTemplate = v8::FunctionTemplate::New(
            env.isolate(), GlobalFunctionCallback, callbackData);

        v8::Local<v8::Context> context = env.isolate()->GetCurrentContext();
        v8::Local<v8::Function> jsFunc = funcTemplate->GetFunction(context).ToLocalChecked();

        return jsFunc;
    }
}
#endif //CONVERTTO_H