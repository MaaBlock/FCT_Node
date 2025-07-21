
#include "headers.h"

namespace FCT {

// FunctionManager 实现 - 现在是非静态的
int64_t FunctionManager::registerFunction(std::unique_ptr<FunctionWrapperBase> wrapper) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int64_t id = m_nextId++;
    m_wrappers[id] = std::move(wrapper);
    return id;
}

void FunctionManager::unregisterFunction(int64_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_wrappers.erase(id);
}

FunctionWrapperBase* FunctionManager::getFunction(int64_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_wrappers.find(id);
    return it != m_wrappers.end() ? it->second.get() : nullptr;
}

void FunctionManager::cleanup() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_wrappers.clear();
}

void GlobalFunctionCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Array> data = args.Data().As<v8::Array>();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Value> idValue = data->Get(context, 0).ToLocalChecked();
    if (!idValue->IsBigInt()) {
        isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Invalid function ID").ToLocalChecked()));
        return;
    }
    int64_t id = idValue.As<v8::BigInt>()->Int64Value();

    v8::Local<v8::Value> envValue = data->Get(context, 1).ToLocalChecked();
    if (!envValue->IsExternal()) {
        isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Invalid environment pointer").ToLocalChecked()));
        return;
    }
    NodeEnvironment* env = static_cast<NodeEnvironment*>(envValue.As<v8::External>()->Value());

    FunctionWrapperBase* wrapper = env->functionManager().getFunction(id);
    if (!wrapper) {
        isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Function wrapper not found").ToLocalChecked()));
        return;
    }

    wrapper->call(args);
}

} // namespace FCT