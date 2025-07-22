/**
* @file JSPromise.h
 * @brief JavaScript Promise wrapper for C++ interoperability
 * @details This file provides a C++ wrapper class for JavaScript Promises,
 *          enabling seamless handling of asynchronous operations between
 *          C++ and JavaScript code with proper state management and result retrieval.
 * @author FCT Team
 */
#ifndef JSPROMISE_H
#define JSPROMISE_H
#include "./ThirdParty.h"
#include <functional>
#include <memory>

namespace FCT {
    class NodeEnvironment;
    /**
      * @brief Enumeration representing the state of a JavaScript Promise
      * @details Mirrors the standard Promise states as defined in ECMAScript specification
      */
    enum class PromiseState {
        pending,    ///< Promise is still pending (not fulfilled or rejected)
        fulfilled,  ///< Promise has been resolved with a value
        rejected    ///< Promise has been rejected with a reason
    };

    /**
    * @brief C++ wrapper for JavaScript Promise objects
    * @details Provides a convenient interface for working with JavaScript Promises
    *          from C++ code, including creation, state monitoring, and result retrieval.
    *          Supports both creating new promises and wrapping existing ones.
    *          Uses std::enable_shared_from_this for safe shared ownership.
    */
    class JSPromise : public std::enable_shared_from_this<JSPromise> {
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

        /**
         * @brief Get local handle to the JavaScript Promise
         * @return Local handle to the Promise
         */
        v8::Local<v8::Promise> getLocalPromise() const;

        /**
         * @brief Get local handle to the Promise resolver
         * @return Local handle to the resolver, or empty handle if not creator
         * @details Only available if this instance created the Promise
         */
        v8::Local<v8::Promise::Resolver> getLocalResolver() const;

        /**
         * @brief Get the current state of the Promise
         * @return Current PromiseState (pending, fulfilled, or rejected)
         */
        PromiseState getState() const;

        /**
         * @brief Check if the Promise is settled (fulfilled or rejected)
         * @return true if Promise is settled, false if still pending
         */
        bool isSettled() const;

        /**
         * @brief Check if the Promise is fulfilled
         * @return true if Promise is fulfilled, false otherwise
         */
        bool isFulfilled() const;

        /**
         * @brief Check if the Promise is rejected
         * @return true if Promise is rejected, false otherwise
         */
        bool isRejected() const;

        /**
         * @brief Get the result value of a fulfilled Promise
         * @tparam T Target C++ type for the result
         * @return Converted result value of type T
         * @details Only valid if Promise is fulfilled. Automatically converts
         *          JavaScript result value to requested C++ type.
         * @throws May throw if Promise is not fulfilled or conversion fails
         */
        template<typename T>
        T getResult() const;

        /**
         * @brief Get the error message of a rejected Promise
         * @return Error message as string
         * @details Only valid if Promise is rejected. Converts JavaScript
         *          rejection reason to a string representation.
         */
        std::string getError() const;

        /**
         * @brief Wait for the Promise to settle
         * @param timeoutMs Timeout in milliseconds (-1 for no timeout)
         * @return true if Promise settled within timeout, false if timeout occurred
         * @details Blocks the current thread until the Promise is settled or timeout occurs.
         *          Use with caution as it can block the calling thread.
         */
        bool wait(int timeoutMs = -1);

        /**
         * @brief Invalidate the Promise wrapper
         * @details Resets all handles and clears environment reference.
         *          Used for cleanup when the environment is being destroyed.
         */
        void invalidate() {
            m_env = nullptr;
            m_promise.Reset();
            m_resolver.Reset();
        }
    };




} // FCT

#endif //JSPROMISE_H
