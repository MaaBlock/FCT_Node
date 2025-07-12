//
// Created by Administrator on 2025/6/7.
//

#ifndef EXPLICITMICROTASKSSCOPE_H
#define EXPLICITMICROTASKSSCOPE_H
namespace FCT {
    class ExplicitMicrotasksScope {
    public:
        explicit ExplicitMicrotasksScope(v8::Isolate* isolate,v8::MicrotaskQueue* queue);
        ~ExplicitMicrotasksScope();

        ExplicitMicrotasksScope(const ExplicitMicrotasksScope&) = delete;
        ExplicitMicrotasksScope& operator=(const ExplicitMicrotasksScope&) = delete;

    private:
        v8::MicrotaskQueue* m_microtaskQueue;
        v8::MicrotasksPolicy m_originalPolicy;
        v8::Isolate* m_isolate;
    };

}
#endif //EXPLICITMICROTASKSSCOPE_H
