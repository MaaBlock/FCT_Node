//
// Created by Administrator on 2025/6/16.
//

#ifndef FCT_NODE_JSPROMISE_HPP
#define FCT_NODE_JSPROMISE_HPP
namespace FCT {
template <typename T>
    T JSPromise::getResult() const
{
    if (!isFulfilled()) {
        throw std::runtime_error("Cannot get result: Promise is not fulfilled");
    }

    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handleScope(m_isolate);

    v8::Local<v8::Context> context = m_env->context();

    v8::Local<v8::Promise> promise = getLocalPromise();
    v8::Local<v8::Value> result = promise->Result();

    return convertFromJS<T>(*m_env, result);
}
}
#endif //JSPROMISES_H
