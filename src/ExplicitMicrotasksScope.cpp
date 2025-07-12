//
// Created by Administrator on 2025/6/7.
//
#include "./headers.h"

namespace FCT {

    ExplicitMicrotasksScope::ExplicitMicrotasksScope(v8::Isolate* isolate, v8::MicrotaskQueue* queue)
    : m_microtaskQueue(queue),m_isolate(isolate),m_originalPolicy(isolate->GetMicrotasksPolicy())
    {
        m_isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    }

    ExplicitMicrotasksScope::~ExplicitMicrotasksScope() {

        m_isolate->SetMicrotasksPolicy(m_originalPolicy);
    }
}
