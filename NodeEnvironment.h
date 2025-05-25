//
// Created by Administrator on 2025/5/22.
//

#ifndef NODEENVIRONMENT_H
#define NODEENVIRONMENT_H

namespace FCT {

    int GetOptimalNodeThreadPoolSize();

    class NodeEnvironment
    {
    protected:
        std::unique_ptr<node::CommonEnvironmentSetup> m_setup;
        v8::Isolate* m_isolate = nullptr;
        node::Environment* m_env = nullptr;
        std::thread m_eventLoopThread;
        bool m_isRunning = false;
        int m_exitCode = 0;
        std::vector<std::string> m_args;
        std::vector<std::string> m_excuteArgs;
        std::vector<std::string> m_modulePaths;
        bool m_environmentInitialized = false;
    public:
        bool setup();
        bool executeArg();
        void stop();
    public:
        NodeEnvironment() = default;
        void args(const std::vector<std::string> args);
        void excuteArgs(const std::vector<std::string> executeArgs);
        void addModulePath(const std::string& path);
        int excute();
        bool execute(std::string_view jsCode);
        std::string base64Encode(const std::string& input);
        void callFunction(const std::string& funcName, const std::vector<v8::Local<v8::Value>>& args);
        void callFunction(const std::string& funcName);
        template<typename ReturnType, typename... Args>
        ReturnType callFunction(const std::string& funcName, Args... args);
    };
    template<typename T>
    T convertFromJS(v8::Isolate* isolate, v8::Local<v8::Value> jsValue) {
        return T();
    }

    template<>
    inline bool convertFromJS<bool>(v8::Isolate* isolate, v8::Local<v8::Value> jsValue) {
        if (jsValue->IsBoolean()) {
            return jsValue->BooleanValue(isolate);
        }
        return false;
    }

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
    inline v8::Local<v8::Value> convertToJS<const char*>(v8::Isolate* isolate, const char* arg) {
        return v8::String::NewFromUtf8(isolate, arg).ToLocalChecked();
    }
    template<typename ReturnType, typename... Args>
    ReturnType NodeEnvironment::callFunction(const std::string& funcName, Args... args)
    {
        if (!m_setup || !m_isolate || !m_env) {
            std::cerr << "Environment not properly set up. Call setup() first." << std::endl;
            return ReturnType();
        }

        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Local<v8::Context> context = m_setup->context();
        v8::Context::Scope context_scope(context);

        v8::Local<v8::Object> global = context->Global();

        v8::Local<v8::String> func_name = v8::String::NewFromUtf8(m_isolate, funcName.c_str(),
                                                                 v8::NewStringType::kNormal).ToLocalChecked();
        v8::MaybeLocal<v8::Value> maybe_func = global->Get(context, func_name);

        if (maybe_func.IsEmpty()) {
            std::cerr << "Function '" << funcName << "' not found in global scope" << std::endl;
            return ReturnType();
        }

        v8::Local<v8::Value> func_val = maybe_func.ToLocalChecked();
        if (!func_val->IsFunction()) {
            std::cerr << "'" << funcName << "' is not a function" << std::endl;
            return ReturnType();
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
            std::cerr << "Error calling function '" << funcName << "'" << std::endl;
            return ReturnType();
        }

        m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);

        return convertFromJS<ReturnType>(m_isolate,result.ToLocalChecked());
    }
} // FCT

#endif //NODEENVIRONMENT_H
