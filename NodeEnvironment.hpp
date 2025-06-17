#ifndef FCT_NODE_NODEENVIRONMENT_HPP
#define FCT_NODE_NODEENVIRONMENT_HPP
namespace FCT
{
    template<typename ReturnType, typename... Args>
    ReturnType NodeEnvironment::callFunction(const std::string& funcName, Args... args)
    {
        if (!m_isolate || !m_env) {
            throw NodeError("callFunction " + funcName + " failed: call setup before use");;
        }

        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Local<v8::Context> context = this->context();
        v8::Context::Scope context_scope(context);

        v8::Local<v8::Object> global = context->Global();

        v8::Local<v8::String> func_name = v8::String::NewFromUtf8(m_isolate, funcName.c_str(),
                                                                 v8::NewStringType::kNormal).ToLocalChecked();
        v8::MaybeLocal<v8::Value> maybe_func = global->Get(context, func_name);

        if (maybe_func.IsEmpty()) {
            throw NodeError("callFunction " + funcName + " failed: function not found");
        }

        v8::Local<v8::Value> func_val = maybe_func.ToLocalChecked();
        if (!func_val->IsFunction()) {
            throw NodeError("callFunction " + funcName + " failed: function is not a function");
        }

        v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(func_val);

        std::vector<v8::Local<v8::Value>> jsArgs = { convertToJS(m_isolate, args)... };

        node::async_context asyncContext = { 0, 0 };
        v8::MaybeLocal<v8::Value> result = node::MakeCallback(
            m_isolate,
            global,
            func,
            static_cast<int>(jsArgs.size()),
            jsArgs.empty() ? nullptr : jsArgs.data(),
            asyncContext);

        if (result.IsEmpty()) {
            throw NodeError("callFunction " + funcName + " failed: JavaSript Error");
        }

        //m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);

        return convertFromJS<ReturnType>(*this,result.ToLocalChecked());
    }
    template<typename... Args>
    JSObject NodeEnvironment::callFunction(const std::string& funcName, Args... args)
    {
        if (!m_isolate || !m_env) {
            throw NodeError("callFunction " + funcName + " failed: call setup before use");;
        }

        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Local<v8::Context> context = this->context();
        v8::Context::Scope context_scope(context);

        v8::Local<v8::Object> global = context->Global();

        v8::Local<v8::String> func_name = v8::String::NewFromUtf8(m_isolate, funcName.c_str(),
                                                                 v8::NewStringType::kNormal).ToLocalChecked();
        v8::MaybeLocal<v8::Value> maybe_func = global->Get(context, func_name);

        if (maybe_func.IsEmpty()) {
            throw NodeError("callFunction " + funcName + " failed: function not found");
        }

        v8::Local<v8::Value> func_val = maybe_func.ToLocalChecked();
        if (!func_val->IsFunction()) {
            throw NodeError("callFunction " + funcName + " failed: function is not a function");
        }

        v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(func_val);

        std::vector<v8::Local<v8::Value>> jsArgs = { convertToJS(m_isolate, args)... };

        node::async_context asyncContext = { 0, 0 };
        v8::MaybeLocal<v8::Value> result = node::MakeCallback(
            m_isolate,
            global,
            func,
            static_cast<int>(jsArgs.size()),
            jsArgs.empty() ? nullptr : jsArgs.data(),
            asyncContext);

        if (result.IsEmpty()) {
            throw NodeError("callFunction " + funcName + " failed: JavaSript Error");
        }

        m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);

        v8::Local<v8::Value> resultValue = result.ToLocalChecked();
        if (resultValue->IsObject()) {
            v8::Local<v8::Object> localObj = resultValue.As<v8::Object>();
            return JSObject(this,m_isolate, localObj);
        } else {
            std::cout << "return wrapper object" << std::endl;

            v8::Local<v8::Object> wrapper = v8::Object::New(m_isolate);
            wrapper->Set(context,
                v8::String::NewFromUtf8(m_isolate, "value").ToLocalChecked(),
                resultValue).Check();

            return JSObject(this,m_isolate, wrapper);
        }
    }
}
#endif