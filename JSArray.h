//
// Created by Administrator on 2025/6/17.
//

#ifndef JSARRAY_H
#define JSARRAY_H
namespace FCT
{
    class JSArray {
    private:
        v8::Isolate* m_isolate;
        v8::Global<v8::Array> m_array;
        NodeEnvironment* m_env;

    public:
        JSArray(NodeEnvironment* env, v8::Isolate* isolate, v8::Global<v8::Array> array)
            : m_env(env), m_isolate(isolate), m_array(std::move(array)) {}

        JSArray(NodeEnvironment* env, v8::Isolate* isolate, v8::Local<v8::Array> array)
            : m_env(env), m_isolate(isolate) {
            m_array.Reset(isolate, array);
        }

        ~JSArray() {
            m_array.Reset();
        }

        JSArray(const JSArray&) = delete;
        JSArray& operator=(const JSArray&) = delete;

        JSArray(JSArray&& other) noexcept
            : m_env(other.m_env), m_isolate(other.m_isolate) {
            m_array.Reset(other.m_isolate, other.getLocalArray());
            other.m_array.Reset();
        }

        JSArray& operator=(JSArray&& other) noexcept {
            if (this != &other) {
                m_array.Reset();
                m_env = other.m_env;
                m_isolate = other.m_isolate;
                m_array.Reset(other.m_isolate, other.getLocalArray());
                other.m_array.Reset();
            }
            return *this;
        }

        v8::Local<v8::Array> getLocalArray() const {
            return m_array.Get(m_isolate);
        }

        JSObject toObject() const {
            v8::Local<v8::Object> obj = getLocalArray().As<v8::Object>();
            return JSObject(m_env, m_isolate, v8::Global<v8::Object>(m_isolate, obj));
        }

        uint32_t length() const {
            v8::Locker locker(m_isolate);
            v8::HandleScope handleScope(m_isolate);
            return getLocalArray()->Length();
        }
        template<typename T>
        T get(uint32_t index) const;

        template<typename T>
        bool set(uint32_t index, const T& value) {
            v8::Locker locker(m_isolate);
            v8::HandleScope handleScope(m_isolate);
            v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
            v8::Local<v8::Array> arr = getLocalArray();

            v8::Local<v8::Value> jsValue = convertToJS(m_isolate, value);
            return arr->Set(context, index, jsValue).FromMaybe(false);
        }

        template<typename T>
        bool push(const T& value) {
            uint32_t currentLength = length();
            return set(currentLength, value);
        }

        class ElementProxy {
        private:
            const JSArray& m_jsArray;
            uint32_t m_index;

        public:
            ElementProxy(const JSArray& jsArray, uint32_t index)
                : m_jsArray(jsArray), m_index(index) {}

            template<typename T>
            operator T() const {
                return m_jsArray.get<T>(m_index);
            }

            template<typename T>
            ElementProxy& operator=(const T& value) {
                const_cast<JSArray&>(m_jsArray).set(m_index, value);
                return *this;
            }
        };

        ElementProxy operator[](uint32_t index) const {
            return ElementProxy(*this, index);
        }

        bool hasIndex(uint32_t index) const {
            return index < length();
        }
    };

}
#endif //JSARRAY_H
