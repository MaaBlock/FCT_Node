#ifndef FCT_NODE_FUNCTIONWRAPPER_HPP
#define FCT_NODE_FUNCTIONWRAPPER_HPP
namespace FCT {
    template <typename ReturnType, typename ... Args>
    void FunctionWrapper<ReturnType, Args...>::call(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = m_env.isolate();
        v8::HandleScope handle_scope(isolate);

        try {
            if (args.Length() < sizeof...(Args)) {
                isolate->ThrowException(v8::Exception::Error(
                    v8::String::NewFromUtf8(isolate, "Not enough arguments").ToLocalChecked()));
                return;
            }

            if constexpr (sizeof...(Args) == 0) {
                if constexpr (std::is_void_v<ReturnType>) {
                    m_function();
                    args.GetReturnValue().SetUndefined();
                } else {
                    auto result = m_function();
                    args.GetReturnValue().Set(convertToJS(m_env, result));
                }
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    callWithArgs(args, std::index_sequence_for<Args...>{});
                    args.GetReturnValue().SetUndefined();
                } else {
                    auto result = callWithArgs(args, std::index_sequence_for<Args...>{});
                    args.GetReturnValue().Set(convertToJS(m_env, result));
                }
            }
        } catch (const std::exception& e) {
            isolate->ThrowException(v8::Exception::Error(
                v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked()));
        }
    }
}
#endif