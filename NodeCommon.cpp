//
// Created by Administrator on 2025/5/22.
//

#include "./headers.h"

namespace FCT {
    std::shared_ptr<node::InitializationResult> NodeCommon::g_initializationResultl = nullptr;
    std::unique_ptr<node::MultiIsolatePlatform> NodeCommon::g_platform;
} // FCT