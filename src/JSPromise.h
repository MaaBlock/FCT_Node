#ifndef JSPROMISE_H
#define JSPROMISE_H
#include "./ThirdParty.h"
#include <functional>
#include <memory>

namespace FCT {
    class NodeEnvironment;

    enum class PromiseState {
        pending,
        fulfilled,
        rejected
    };

    class JSPromise {
    private:
        v8::Isolate* m_isolate;
        NodeEnvironment* m_env;
        v8::Global<v8::Promise> m_promise;
        v8::Global<v8::Promise::Resolver> m_resolver;
        bool m_isCreator;

    public:
        JSPromise(NodeEnvironment* env, v8::Isolate* isolate);

        JSPromise(NodeEnvironment* env, v8::Isolate* isolate, v8::Local<v8::Promise> promise);

        ~JSPromise();

        JSPromise(JSPromise&& other) noexcept;

        JSPromise& operator=(JSPromise&& other) noexcept;

        JSPromise(const JSPromise&) = delete;
        JSPromise& operator=(const JSPromise&) = delete;

        v8::Local<v8::Promise> getLocalPromise() const;

        v8::Local<v8::Promise::Resolver> getLocalResolver() const;

        PromiseState getState() const;

        bool isSettled() const;

        bool isFulfilled() const;

        bool isRejected() const;

        template<typename T>
        T getResult() const;

        std::string getError() const;

        bool wait(int timeoutMs = -1);
    };


} // FCT

#endif //JSPROMISE_H
