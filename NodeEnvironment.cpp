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

        std::cout << "V8 Isolate, event loop and context initialized successfully" << std::endl;
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
                while (!pThis->m_embedSem)
                {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1));
                }
                pThis->m_embedSem = false;
                pThis->pollEvents();
                pThis->weakMainThread();
            }
        }, this);
        /*
        m_pollThread = std::thread([this]()
        {
           while (m_pollThreadRunning)
            {
                while (!m_embedSem)
                {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1));
                }
                m_embedSem = false;
                pollEvents();
                weakMainThread();
            }
        });*/
        runEventLoopOnce();
    }

    void NodeEnvironment::excuteSetupJSCode()
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Context::Scope context_scope(this->context());

        std::string setupModulePaths = "";
        for (const auto& path : m_modulePaths) {
            setupModulePaths += "module.paths.push('" + path + "');\n";
        }

        std::string initCode = R"(
const publicRequire = require('node:module').createRequire(process.cwd() + '/');
globalThis.require = publicRequire;

)" + setupModulePaths + R"(

console.log('module:',module);

globalThis.__FCT_executeScriptBase64 = function(codeBase64) {
    const vm = require('node:vm');
    try {
        const code = Buffer.from(codeBase64, 'base64').toString('utf8');
        const result = vm.runInThisContext(code, {
            filename: 'user_script.js',
            displayErrors: true
        });
        return result;
    } catch (error) {
        console.error('Error executing script:', error);
        return { error: error.message };
    }
};
globalThis.__FCT_executeScriptString = function(code) {
    const vm = require('node:vm');
    try {
        const result = vm.runInThisContext(code, {
            filename: 'user_script.js',
            displayErrors: true
        });
        return result;
    } catch (error) {
        console.error('Error executing script:', error);
        return { error: error.message };
    }
};

console.log('FCT Node.js environment initialized');
)";

        /*v8::MaybeLocal<v8::Value> init_ret = node::LoadEnvironment(m_env, initCode);

        if (init_ret.IsEmpty()) {
            std::cerr << "Failed to initialize JavaScript environment" << std::endl;
            return;
        }*/

        switch (m_codeFrom)
        {
        case CodeFrom::file:

            break;
        case CodeFrom::arg:
            {
                std::string executeCode = "globalThis.__FCT_executeScriptString(process.argv[" +
                                         std::to_string(m_codeArgIndex) + "]);";
                v8::MaybeLocal<v8::Value> exec_ret = node::LoadEnvironment(m_env, executeCode);
                if (!exec_ret.IsEmpty()) {
                    v8::Local<v8::Value> localResult = exec_ret.ToLocalChecked();
                    m_setupRet.Reset(m_isolate, localResult);
                } else {
                    std::cerr << "Failed to execute JavaScript code from arguments" << std::endl;
                    return;
                }
                m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);
            }
            break;
        case CodeFrom::string:
            {
                std::string jsCodeStr(m_jsCode);
                std::string base64Code = base64Encode(jsCodeStr);

                std::string executeCode = "globalThis.__FCT_executeScriptBase64('" + base64Code + "');";

                v8::MaybeLocal<v8::Value> exec_ret = node::LoadEnvironment(m_env, initCode + executeCode);
                v8::Local<v8::Value> localResult = exec_ret.ToLocalChecked();
                m_setupRet.Reset(m_isolate,localResult);
                break;
            }
        }

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

    bool NodeEnvironment::setup()
    {
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

        v8::Local<v8::Context> context = this->context();

        v8::Context::Scope ContextScope(context);
        m_isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);

        m_env = node::CreateEnvironment(isolateData,context,m_args,m_excuteArgs,node::EnvironmentFlags::kOwnsProcessState);
        /*
        std::vector<std::string> errors;

        m_setup = node::CommonEnvironmentSetup::Create(platform, &errors, args, exec_args);
        if (!m_setup) {
            for (const std::string& err : errors)
                fprintf(stderr, "%s: %s\n", args[0].c_str(), err.c_str());
            return false;
        }

        m_isolate = m_setup->isolate();
        m_env = m_setup->env();*/
        excuteSetupJSCode();


        //beginPollThread();
        return true;
    }

std::string NodeEnvironment::base64Encode(const std::string& input) {
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
    if (!m_env) {
        return;
    }

    node::Stop(m_env);
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