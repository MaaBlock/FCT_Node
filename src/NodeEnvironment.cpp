//
// Created by Administrator on 2025/5/22.
//

#include "./headers.h"

namespace FCT {
    int GetOptimalNodeThreadPoolSize(){
        unsigned int hardware_concurrency = std::thread::hardware_concurrency();

        if (hardware_concurrency == 0) {
            hardware_concurrency = 4;
        }

        return hardware_concurrency;
    }

    void NodeEnvironment::args(const std::vector<std::string> args)
    {
        m_args = args;
    }

    void NodeEnvironment::excuteArgs(const std::vector<std::string> executeArgs)
    {
        m_excuteArgs = executeArgs;
    }

    void NodeEnvironment::addModulePath(const std::string& path)
    {
        m_modulePaths.push_back(path);
    }

    void NodeEnvironment::init()
    {
        auto platform = NodeCommon::GetPlatform().get();
        if (!platform) {
            std::cerr << "Failed to get Node.js platform" << std::endl;
            return;
        }

        uv_loop_t* loop = new uv_loop_t;
        int result = uv_loop_init(loop);
        if (result != 0) {
            std::cerr << "Failed to initialize event loop: " << uv_strerror(result) << std::endl;
            delete loop;
            return;
        }
        m_loop = loop;

        m_arrayBufferAllocator = node::ArrayBufferAllocator::Create();

        m_isolate = node::NewIsolate(m_arrayBufferAllocator.get(),m_loop,platform);
        if (!m_isolate) {
            std::cerr << "Failed to create V8 Isolate" << std::endl;
            uv_loop_close(m_loop);
            delete m_loop;
            m_loop = nullptr;
            return;
        }


        m_isolate->SetData(0, this);


        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);

        v8::Local<v8::Context> context = node::NewContext(m_isolate);

        m_context.Reset(m_isolate, context);
    }

    void NodeEnvironment::weakMainThread()
    {
        NodeEnvTicker* ticker = new NodeEnvTicker();
        ticker->cb = [this,ticker]()
        {
            v8::Locker locker(m_isolate);
            v8::Isolate::Scope isolate_scope(m_isolate);
            v8::HandleScope handle_scope(m_isolate);
            v8::Context::Scope context_scope(this->context());
            this->runEventLoopOnce();
            m_embedSem = true;
            delete ticker;
        };
        m_tickerQueue.enqueue(ticker);
    }

    void NodeEnvironment::beginPollThread()
    {
        m_embedSem = false;
        m_pollThreadRunning = true;
        uv_thread_create(&m_pollThreadId, [](void* arg)
        {
            auto* pThis = static_cast<NodeEnvironment*>(arg);
            while (pThis->m_pollThreadRunning)
            {
                while (!pThis->m_embedSem && pThis->m_pollThreadRunning)
                {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1));
                }
                pThis->m_embedSem = false;
                pThis->pollEvents();
                pThis->weakMainThread();
            }
        }, this);
        runEventLoopOnce();
    }

    void NodeEnvironment::excuteSetupJSCode()
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Context::Scope context_scope(this->context());

        std::string initCode =
            R"(
const Module = require('module')
const original_nodeModulePaths = Module._nodeModulePaths;
const customModulePaths = [
)" +
        [&]() {
            std::string paths = "";
            for (size_t i = 0; i < m_modulePaths.size(); ++i) {
                std::string encodedPath = base64Encode(m_modulePaths[i]);
                paths += "    Buffer.from('" + encodedPath + "', 'base64').toString('utf8')";
                if (i < m_modulePaths.size() - 1) {
                    paths += ",\n";
                }
            }
            return paths;
        }() +
        R"(
];
Module._nodeModulePaths = function(from) {
    const originalPaths = original_nodeModulePaths.call(this, from);

    const newPaths = [...originalPaths, ...customModulePaths];
    return newPaths;
};

const publicRequire = require('node:module').createRequire(process.cwd() + '/');
globalThis.require = publicRequire;
)";
        v8::MaybeLocal<v8::Value> exec_ret = node::LoadEnvironment(m_env, initCode);
        beginPollThread();
    }

    void NodeEnvironment::stopEventLoop() {
        m_eventLoopRunning.store(false);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

    }

    void NodeEnvironment::runEventLoopOnce()
    {

        node::Environment* env = m_env;

        if (!env)
            return;


        v8::Isolate::Scope IsolateScope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        auto context = this->context();

        v8::Context::Scope context_scope(context);

        v8::TryCatch TryCatch(m_isolate);
        {
            //ExplicitMicrotasksScope microtasksScope(m_isolate,this->context()->GetMicrotaskQueue());
            //context->GetMicrotaskQueue()->PerformCheckpoint(m_isolate);
            m_isolate->PerformMicrotaskCheckpoint();

            int r = uv_run(m_loop,UV_RUN_NOWAIT );
           NodeCommon::GetPlatform()->DrainTasks(m_isolate);

            m_embedSem = true;
        }
    }

    void NodeEnvironment::excuteScript(const std::string& jsCode)
    {
        if (!m_isolate || !m_env) {
            std::cerr << "Environment not properly set up. Call setup() first." << std::endl;
            return;
        }

        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Local<v8::Context> context = this->context();
        v8::Context::Scope context_scope(context);

        v8::TryCatch try_catch(m_isolate);

        v8::Local<v8::String> source = v8::String::NewFromUtf8(
            m_isolate,
            jsCode.c_str(),
            v8::NewStringType::kNormal
        ).ToLocalChecked();

        std::string script_name = "user_script.js";
        v8::Local<v8::String> name = v8::String::NewFromUtf8(
            m_isolate,
            script_name.c_str(),
            v8::NewStringType::kNormal
        ).ToLocalChecked();

        v8::ScriptOrigin origin(name);

        v8::MaybeLocal<v8::Script> maybe_script = v8::Script::Compile(context, source, &origin);
        if (maybe_script.IsEmpty()) {
            if (try_catch.HasCaught()) {
                v8::String::Utf8Value error(m_isolate, try_catch.Exception());
                std::cerr << "Script compilation failed: " << *error << std::endl;

                v8::Local<v8::Message> message = try_catch.Message();
                if (!message.IsEmpty()) {
                    v8::String::Utf8Value filename_utf8(m_isolate, message->GetScriptOrigin().ResourceName());
                    int linenum = message->GetLineNumber(context).FromMaybe(-1);
                    std::cerr << "  at " << *filename_utf8 << ":" << linenum << std::endl;
                }
            }
            return;
        }

        v8::Local<v8::Script> script = maybe_script.ToLocalChecked();

        v8::MaybeLocal<v8::Value> maybe_result = script->Run(context);
        if (maybe_result.IsEmpty()) {
            if (try_catch.HasCaught()) {
                v8::String::Utf8Value error(m_isolate, try_catch.Exception());
                std::cerr << "Script execution failed: " << *error << std::endl;

                v8::Local<v8::Message> message = try_catch.Message();
                if (!message.IsEmpty()) {
                    v8::String::Utf8Value filename_utf8(m_isolate, message->GetScriptOrigin().ResourceName());
                    int linenum = message->GetLineNumber(context).FromMaybe(-1);
                    std::cerr << "  at " << *filename_utf8 << ":" << linenum << std::endl;

                    v8::Local<v8::Value> stack_trace = try_catch.StackTrace(context).ToLocalChecked();
                    if (!stack_trace.IsEmpty()) {
                        v8::String::Utf8Value stack_utf8(m_isolate, stack_trace);
                        std::cerr << "Stack trace:\n" << *stack_utf8 << std::endl;
                    }
                }
            }
            return;
        }

        m_isolate->PerformMicrotaskCheckpoint();
    }

    void NodeEnvironment::cleanup()
    {
        for (JSPromise* promise : m_promises)
        {
            if (promise) {
                promise->invalidate();
            }
        }
        m_promises.clear();

        if (m_pollThreadRunning) {
            m_pollThreadRunning = false;
            if (m_pollThreadId) {
                uv_thread_join(&m_pollThreadId);
            }
        }

        if (m_env) {
            v8::Locker locker(m_isolate);
            v8::Isolate::Scope isolate_scope(m_isolate);
            v8::HandleScope handle_scope(m_isolate);
            node::Stop(m_env);
            node::FreeEnvironment(m_env);
            m_env = nullptr;
        }

        if (m_isolateData) {
            node::FreeIsolateData(m_isolateData);
            m_isolateData = nullptr;
        }

        if (!m_context.IsEmpty()) {
            m_context.Reset();
        }

        if (m_isolate) {
            m_isolate->Dispose();
            m_isolate = nullptr;
        }

        if (m_loop) {
            uv_loop_close(m_loop);
            delete m_loop;
            m_loop = nullptr;
        }
    }

    bool NodeEnvironment::setup()
    {
        init();
        m_functionManager = new FunctionManager();
        auto platform = NodeCommon::GetPlatform().get();
        auto args = m_args;
        auto exec_args = m_excuteArgs;

        for (auto &arg : args)
        {
            std::cout << "Args: " << arg << std::endl;
        }
        for (auto &arg : exec_args)
        {
            std::cout << "Exec Args: " << arg << std::endl;
        }

        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);

        auto* isolateData = node::CreateIsolateData(m_isolate, m_loop, platform);
        m_isolateData = isolateData;
        v8::Local<v8::Context> context = this->context();

        v8::Context::Scope ContextScope(context);
        m_isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);

        m_env = node::CreateEnvironment(isolateData,context,m_args,m_excuteArgs,node::EnvironmentFlags::kOwnsProcessState);
        excuteSetupJSCode();
        return true;
    }

    std::string NodeEnvironment::base64Encode(const std::string& input)
    {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        for (char c : input) {
            char_array_3[i++] = c;
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];

            while((i++ < 3))
                ret += '=';
        }

        return ret;
    }
    void NodeEnvironment::pollEvents() {

#ifdef _WIN32
        DWORD bytes, timeout;
        ULONG_PTR key;
        OVERLAPPED* overlapped;

        timeout = uv_backend_timeout(m_loop);

        GetQueuedCompletionStatus(m_loop->iocp, &bytes, &key, &overlapped, timeout);

        if (overlapped != nullptr)
            PostQueuedCompletionStatus(m_loop->iocp, bytes, key, overlapped);
#else

#endif
    }
    void NodeEnvironment::blockRunLoop()
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Context::Scope context_scope(this->context());
        while (true)
        {
            pollEvents();
            runEventLoopOnce();
        }
    }

    JSObject NodeEnvironment::global()
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        auto context = this->context();
        v8::Context::Scope context_scope(context);

        v8::Local<v8::Object> globalObj = context->Global();
        return JSObject(this,m_isolate, globalObj);
    }
    void NodeEnvironment::callFunction(const std::string& funcName, const std::vector<v8::Local<v8::Value>>& args)
    {
        if (!m_isolate || !m_env) {
            std::cerr << "Environment not properly set up. Call setup() first." << std::endl;
            return;
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
            std::cerr << "Function '" << funcName << "' not found in global scope" << std::endl;
            return;
        }

        v8::Local<v8::Value> func_val = maybe_func.ToLocalChecked();
        if (!func_val->IsFunction()) {
            std::cerr << "'" << funcName << "' is not a function" << std::endl;
            return;
        }

        v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(func_val);

        std::vector<v8::Local<v8::Value>> argv = args;

        node::async_context asyncContext = { 0, 0 };
        v8::MaybeLocal<v8::Value> result = node::MakeCallback(
            m_isolate,
            global,
            func,
            static_cast<int>(argv.size()),
            argv.empty() ? nullptr : argv.data(),
            asyncContext);

        if (result.IsEmpty()) {
            std::cerr << "Error calling function '" << funcName << "'" << std::endl;
            return;
        }

        m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);
    }
    void NodeEnvironment::callFunction(const std::string& funcName)
    {
        std::vector<v8::Local<v8::Value>> args;
        callFunction(funcName, args);
    }

    void NodeEnvironment::stop()
    {
        cleanup();
    }


    void NodeEnvironment::tick()
    {
        NodeEnvTicker* ticker;
        while (m_tickerQueue.try_dequeue(ticker))
        {
            ticker->cb();
        }
    }
} // FCT