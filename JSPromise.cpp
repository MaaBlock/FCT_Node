//
// Created by Administrator on 2025/6/1.
//

#include "JSPromise.h"

namespace FCT {
    JSPromise::JSPromise(NodeEnvironment* env, v8::Isolate* isolate): m_isolate(isolate), m_env(env), m_isCreator(true)
    {
        v8::Locker locker(m_isolate);
        v8::HandleScope handleScope(m_isolate);
        v8::Local<v8::Context> context = m_isolate->GetCurrentContext();

        v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
        m_resolver.Reset(m_isolate, resolver);

        v8::Local<v8::Promise> promise = resolver->GetPromise();
        m_promise.Reset(m_isolate, promise);
    }

    JSPromise::JSPromise(NodeEnvironment* env, v8::Isolate* isolate, v8::Local<v8::Promise> promise): m_isolate(isolate), m_env(env), m_isCreator(false)
    {
        m_promise.Reset(m_isolate, promise);
    }

    JSPromise::~JSPromise()
    {
        m_resolver.Reset();
        m_promise.Reset();
    }

    JSPromise::JSPromise(JSPromise&& other) noexcept: m_isolate(other.m_isolate), m_env(other.m_env), m_isCreator(other.m_isCreator)
    {
        if (!other.m_resolver.IsEmpty()) {
            m_resolver.Reset(other.m_isolate, other.getLocalResolver());
            other.m_resolver.Reset();
        }

        m_promise.Reset(other.m_isolate, other.getLocalPromise());
        other.m_promise.Reset();
    }

    JSPromise& JSPromise::operator=(JSPromise&& other) noexcept
    {
        if (this != &other) {
            m_resolver.Reset();
            m_promise.Reset();

            m_isolate = other.m_isolate;
            m_env = other.m_env;
            m_isCreator = other.m_isCreator;

            if (!other.m_resolver.IsEmpty()) {
                m_resolver.Reset(other.m_isolate, other.getLocalResolver());
                other.m_resolver.Reset();
            }

            m_promise.Reset(other.m_isolate, other.getLocalPromise());
            other.m_promise.Reset();
        }
        return *this;
    }

    v8::Local<v8::Promise> JSPromise::getLocalPromise() const
    {
        return m_promise.IsEmpty() ? v8::Local<v8::Promise>() : m_promise.Get(m_isolate);
    }

    v8::Local<v8::Promise::Resolver> JSPromise::getLocalResolver() const
    {
        return m_resolver.IsEmpty() ? v8::Local<v8::Promise::Resolver>() : m_resolver.Get(m_isolate);
    }

    PromiseState JSPromise::getState() const
    {
        v8::Locker locker(m_isolate);
        v8::HandleScope handleScope(m_isolate);

        v8::Local<v8::Promise> promise = getLocalPromise();
        if (promise.IsEmpty()) {
            return PromiseState::pending;
        }

        if (promise->State() == v8::Promise::kPending) {
            return PromiseState::pending;
        } else if (promise->State() == v8::Promise::kFulfilled) {
            return PromiseState::fulfilled;
        } else { // v8::Promise::kRejected
            return PromiseState::rejected;
        }
    }

    bool JSPromise::isSettled() const
    {
        PromiseState state = getState();
        return state == PromiseState::fulfilled || state == PromiseState::rejected;
    }

    bool JSPromise::isFulfilled() const
    {
        return getState() == PromiseState::fulfilled;
    }

    bool JSPromise::isRejected() const
    {
        return getState() == PromiseState::rejected;
    }

    std::string JSPromise::getError() const
    {
        if (!isRejected()) {
            throw std::runtime_error("Cannot get error: Promise is not rejected");
        }

        v8::Locker locker(m_isolate);
        v8::HandleScope handleScope(m_isolate);

        v8::Local<v8::Promise> promise = getLocalPromise();
        v8::Local<v8::Value> result = promise->Result();

        v8::String::Utf8Value utf8Value(m_isolate, result);
        return std::string(*utf8Value, utf8Value.length());
    }

    bool JSPromise::wait(int timeoutMs)
    {
        auto startTime = std::chrono::steady_clock::now();

        while (!isSettled()) {

            if (timeoutMs >= 0) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
                if (elapsed >= timeoutMs) {
                    return false;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        return true;
    }
} // FCT