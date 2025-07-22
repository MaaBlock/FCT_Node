/**
* @file NodeEnvironment.h
 * @brief Node.js environment management and JavaScript execution interface
 * @details This file provides the main NodeEnvironment class for managing Node.js runtime,
 *          executing JavaScript code, and handling C++/JavaScript interoperability.
 *          Users should not include this file directly, but rather include FCT_Node.h instead.
 * @author FCT Team
 * @date 2025/5/22
 */
#ifndef NODEENVIRONMENT_H
#define NODEENVIRONMENT_H
#include "FunctionWrapper.h"

#include "JSObject.h"
namespace FCT {
    class JSPromise;
    /*todo:
 *  1. 完成CodeFrom
 *
 *
 */
    /**
     * @brief Get optimal thread pool size for Node.js
     * @return Number of CPU cores available for optimal thread pool sizing
     */
    int GetOptimalNodeThreadPoolSize();
    /**
     * @brief Enumeration for JavaScript code source types
     * @deprecated This enum is deprecated and no longer used
     */
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
        FunctionManager* m_functionManager;
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
        void cleanup();
        bool executeArg();
        void init();
        void code(std::string jsCode)
        {
            m_codeFrom = CodeFrom::string;
            m_jsCode = jsCode;
        }
        void processEvents(v8::Locker& locker, v8::Isolate::Scope& isolate_scope, v8::HandleScope& handle_scope,
                           v8::Context::Scope& context_scope);
        int excute();
        void pollEvents();
        void PollEvents();
        void blockRunLoop();
        bool execute(std::string_view jsCode);
    public:
        FunctionManager& functionManager() { return *m_functionManager; }
        /**
         * @brief Execute JavaScript code
         * @param jsCode JavaScript code string to execute
         */
        void excuteScript(const std::string& jsCode);
        /**
         * @brief Setup and initialize the Node.js environment
         * @return true if setup successful, false otherwise
         */
        bool setup();
        /**
         * @brief Stop the Node.js environment
         * @details Gracefully stops the event loop and cleans up resources
         */
        void stop();
        /**
        * @brief Process one tick of the event loop
        * @details Should be called regularly to handle asynchronous operations
        */
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
        /**
        * @brief Set command line arguments
        * @param args Vector of argument strings
        */
        void args(const std::vector<std::string> args);
        /**
         * @brief Set execution arguments
         * @param executeArgs Vector of execution argument strings
         */
        void excuteArgs(const std::vector<std::string> executeArgs);
        /**
        * @brief Add module search path
        * @param path Path to add to module search paths
        * @example
        * @code
        * env.addModulePath("./node_modules");
        * env.addModulePath("F:/node_modules");
        * @endcode
        */
        void addModulePath(const std::string& path);
        /**
         * @brief Encode string to base64
         * @param input Input string to encode
         * @return Base64 encoded string
         */
        std::string base64Encode(const std::string& input);
        /**
         * @brief Call JavaScript function with V8 values
         * @param funcName Function name to call
         * @param args Vector of V8 values as arguments
         */
        void callFunction(const std::string& funcName, const std::vector<v8::Local<v8::Value>>& args);
        /**
         * @brief Call JavaScript function without arguments
         * @param funcName Function name to call
         */
        void callFunction(const std::string& funcName);
        /**
       * @brief Call JavaScript function with typed return value and arguments
       * @tparam ReturnType Type of return value
       * @tparam Args Types of function arguments
       * @param funcName Function name to call
       * @param args Function arguments
       * @return Function return value converted to ReturnType
       * @example
       * @code
       * std::string result = env.callFunction<std::string>("foo");
       * int sum = env.callFunction<int>("add", 5, 3);
       * @endcode
       */
        template<typename ReturnType, typename... Args>
        ReturnType callFunction(const std::string& funcName, Args... args);
        /**
        * @brief Call JavaScript function returning JSObject
        * @tparam Args Types of function arguments
        * @param funcName Function name to call
        * @param args Function arguments
        * @return JSObject wrapping the return value
        */
        template<typename... Args>
        JSObject callFunction(const std::string& funcName, Args... args);
        /**
         * @brief Get V8 isolate instance
         * @return Pointer to V8 isolate
         */
        v8::Isolate* isolate() const { return m_isolate; }
        /**
         * @brief Get V8 context
         * @return Local handle to V8 context
         */
        v8::Local<v8::Context> context() const
        {
            return v8::Local<v8::Context>::New(m_isolate, m_context);
        }
        /**
         * @brief Get global JavaScript object
         * @return JSObject wrapping the global object
         * @example
         * @code
         * env.global()["cppAdd"] = [](int a, int b) {
         *     return a + b;
         * };
         * @endcode
         */
        JSObject global();
    };

} // FCT
//todo:封装一个 jobeject，然后支持  obj[property] 来访问字段
#endif //NODEENVIRONMENT_H
