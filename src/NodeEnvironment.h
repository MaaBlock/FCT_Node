//
// Created by Administrator on 2025/5/22.
//

#ifndef NODEENVIRONMENT_H
#define NODEENVIRONMENT_H
#include "JSObject.h"
namespace FCT {
    class JSPromise;
    /*todo:
 *  1. 完成CodeFrom
 *
 *
 */

    int GetOptimalNodeThreadPoolSize();
    enum class CodeFrom
    {
        file,
        arg,
        string
    };
    struct NodeEnvTicker
    {
        std::function<void()> cb;
    };
    class NodeEnvironment
    {
    protected:
        v8::Isolate* m_isolate = nullptr;
        node::Environment* m_env = nullptr;
        std::thread m_eventLoopThread;
        bool m_isRunning = false;
        int m_exitCode = 0;
        std::vector<std::string> m_args;
        std::vector<std::string> m_excuteArgs;
        std::vector<std::string> m_modulePaths;
        bool m_environmentInitialized = false;
        CodeFrom m_codeFrom = CodeFrom::arg;
        std::string m_jsCode;
        uint32_t m_codeArgIndex = 1;
        std::string m_codeFilePath;
        std::string m_setupJSCode;
        v8::Global<v8::Value> m_setupRet;

        std::atomic<bool> m_eventLoopRunning{false};
        moodycamel::ConcurrentQueue<NodeEnvTicker*> m_tickerQueue;
        std::thread m_pollThread;
        bool m_pollThreadRunning = false;
        std::atomic<bool> m_embedSem = false;
        uv_sem_t m_uvEmbedSem;
        uv_loop_t* m_loop = nullptr;

        v8::Global<v8::Context> m_context;
        std::unique_ptr<node::ArrayBufferAllocator> m_arrayBufferAllocator;
        uv_thread_t m_pollThreadId;
        node::IsolateData* m_isolateData;
        std::vector<JSPromise*> m_promises;
    protected:
        void weakMainThread();
        void beginPollThread();
        void generateSetupJSCode();
        void excuteSetupJSCode();
        void processEvents(int timeoutMs);
        void runEventLoopInThread();
        void runEventLoopInThread(int timeoutMs);
        void stopEventLoop();
        void runEventLoopOnce();
    public:
        void excuteScript(const std::string& jsCode);
        void cleanup();
        bool setup();
        bool executeArg();
        void stop();
        void tick();
        void registerPromise(JSPromise* promise) {
            m_promises.push_back(promise);
        }
        void unregisterPromise(JSPromise* promise) {
            m_promises.erase(
                std::remove(m_promises.begin(), m_promises.end(), promise),
                m_promises.end()
            );
        }
    public:
        void init();
        void code(std::string jsCode)
        {
            m_codeFrom = CodeFrom::string;
            m_jsCode = jsCode;
        }
        void args(const std::vector<std::string> args);
        void excuteArgs(const std::vector<std::string> executeArgs);
        void addModulePath(const std::string& path);
        void processEvents(v8::Locker& locker, v8::Isolate::Scope& isolate_scope, v8::HandleScope& handle_scope,
                           v8::Context::Scope& context_scope);
        int excute();
        bool execute(std::string_view jsCode);
        std::string base64Encode(const std::string& input);
        void pollEvents();
        void PollEvents();
        void blockRunLoop();
        void callFunction(const std::string& funcName, const std::vector<v8::Local<v8::Value>>& args);
        void callFunction(const std::string& funcName);
        template<typename ReturnType, typename... Args>
        ReturnType callFunction(const std::string& funcName, Args... args);
        template<typename... Args>
        JSObject callFunction(const std::string& funcName, Args... args);
        v8::Isolate* isolate() const { return m_isolate; }
        v8::Local<v8::Context> context() const
        {
            return v8::Local<v8::Context>::New(m_isolate, m_context);
        }
    };

} // FCT
//todo:封装一个 jobeject，然后支持  obj[property] 来访问字段
#endif //NODEENVIRONMENT_H
