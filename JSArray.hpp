//
// Created by Administrator on 2025/6/17.
//

#ifndef FCT_NODE_JSARRAY_HPP
#define FCT_NODE_JSARRAY_HPP
namespace FCT {
    template <typename T>
    T JSArray::get(uint32_t index) const
    {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolateScope(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_env->context();
        v8::Context::Scope contextScope(context);
        v8::Local<v8::Array> arr = getLocalArray();

        v8::MaybeLocal<v8::Value> maybeValue = arr->Get(context, index);
        if (maybeValue.IsEmpty()) {
            throw std::runtime_error("Failed to get array element at index " + std::to_string(index));
        }

        v8::Local<v8::Value> value = maybeValue.ToLocalChecked();
        return convertFromJS<T>(*m_env, value);
    }
}

#endif //JSARRAY__H
