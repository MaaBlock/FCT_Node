#ifndef JSOBJECT_HPP
#define JSOBJECT_HPP
namespace FCT
{
    template <typename T>
    T JSObject::get(const std::string& propertyName) const
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_env->context();
        v8::Context::Scope context_scope(context);
        v8::Local<v8::Object> obj = getLocalObject();
        if (obj.IsEmpty())
        {
            std::cout << "Object is empty." << std::endl;
        }
        v8::Local<v8::String> key = v8::String::NewFromUtf8(m_isolate, propertyName.c_str()).ToLocalChecked();
        v8::MaybeLocal<v8::Value> maybeValue = obj->Get(context, key);

        if (maybeValue.IsEmpty()) {
            throw std::runtime_error("Property not found: " + propertyName);
        }

        v8::Local<v8::Value> value = maybeValue.ToLocalChecked();
        return convertFromJS<T>(*m_env, value);
    }

    template <typename T>
    bool JSObject::set(const std::string& propertyName, const T& value)
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_env->context();
        v8::Context::Scope context_scope(context);
        v8::Local<v8::Object> obj = getLocalObject();

        v8::Local<v8::String> key = v8::String::NewFromUtf8(m_isolate, propertyName.c_str()).ToLocalChecked();
        v8::Local<v8::Value> jsValue = convertToJS(*m_env, value);

        return obj->Set(context, key, jsValue).FromMaybe(false);
    }

    template<>
    inline v8::Local<v8::Value> convertToJS<JSObject>(NodeEnvironment& isolate, const JSObject& arg) {
        return arg.getLocalObject();
    }

    template<typename ReturnType, typename... Args>
    ReturnType JSObject::call(const std::string& funcName, Args... args) {
        v8::Isolate* isolate = m_isolate;
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = m_env->context();
        v8::Context::Scope context_scope(context);

        v8::Local<v8::Object> obj = getLocalObject();
        v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, funcName.c_str()).ToLocalChecked();
        v8::Local<v8::Value> funcVal;

        bool hasFunc = obj->Get(context, name).ToLocal(&funcVal) && funcVal->IsFunction();

        if (!hasFunc) {
            if constexpr (std::is_void_v<ReturnType>) return;
            else return ReturnType();
        }

        v8::Local<v8::Function> func = funcVal.As<v8::Function>();
        std::vector<v8::Local<v8::Value>> argv = { convertToJS(*m_env, args)... };

        v8::MaybeLocal<v8::Value> maybeResult = func->Call(context, obj, argv.size(), argv.data());

        if constexpr (std::is_void_v<ReturnType>) {
            return;
        } else {
            if (!maybeResult.IsEmpty()) {
                return convertFromJS<ReturnType>(*m_env, maybeResult.ToLocalChecked());
            }
            return ReturnType();
        }
    }
}
#endif