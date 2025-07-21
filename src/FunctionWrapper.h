#ifndef FCT_NODE_FUNCTIONWRAPPER_H
#define FCT_NODE_FUNCTIONWRAPPER_H

#include <v8.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace FCT
{
    class FunctionWrapperBase {
    public:
        virtual ~FunctionWrapperBase() = default;
        virtual void call(const v8::FunctionCallbackInfo<v8::Value>& args) = 0;
    };

    class NodeEnvironment;
    template<typename ReturnType, typename... Args>
    class FunctionWrapper : public FunctionWrapperBase {
    private:
        std::function<ReturnType(Args...)> m_function;
        NodeEnvironment& m_env;
    public:
        FunctionWrapper(std::function<ReturnType(Args...)> func,NodeEnvironment& env)
            : m_function(std::move(func)),m_env(env) {}

        void call(const v8::FunctionCallbackInfo<v8::Value>& args) override;

    private:
        template<std::size_t... I>
        auto callWithArgs(const v8::FunctionCallbackInfo<v8::Value>& args,
                         std::index_sequence<I...>) {
            if constexpr (std::is_void_v<ReturnType>) {
                m_function(convertFromJS<Args>(m_env, args[I])...);
            } else {
                return m_function(convertFromJS<Args>(m_env, args[I])...);
            }
        }
    };


    class FunctionManager {
    private:
        std::unordered_map<int64_t, std::unique_ptr<FunctionWrapperBase>> m_wrappers;
        std::mutex m_mutex;
        int64_t m_nextId = 1;

    public:
        int64_t registerFunction(std::unique_ptr<FunctionWrapperBase> wrapper);
        void unregisterFunction(int64_t id);
        FunctionWrapperBase* getFunction(int64_t id);
        void cleanup();
    };


    void GlobalFunctionCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace FCT

#endif // FCT_NODE_FUNCTIONWRAPPER_H