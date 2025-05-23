//
// Created by Administrator on 2025/5/22.
//

#ifndef NODECOMMON_H
#define NODECOMMON_H
#include "./NodeEnvironment.h"

namespace FCT {

    class NodeCommon
    {
    private:
        static std::shared_ptr<node::InitializationResult> g_initializationResultl;
        static std::unique_ptr<node::MultiIsolatePlatform> g_platform;
    public:
        static std::shared_ptr<node::InitializationResult> GetInitializationResult() {
            return g_initializationResultl;
        }
        static const std::unique_ptr<node::MultiIsolatePlatform>& GetPlatform()
        {
            return g_platform;
        }
        static void Init()
        {
            auto args = std::vector<std::string>();
            args.push_back("FCTApp");
            std::shared_ptr<node::InitializationResult> result =
                node::InitializeOncePerProcess(args, {
                  node::ProcessInitializationFlags::kNoInitializeV8,
                  node::ProcessInitializationFlags::kNoInitializeNodeV8Platform
                });
            for (const std::string& error : result->errors())
            {
                std::cout << error << std::endl;
            }
            if (result->early_return() != 0) {
                throw std::runtime_error(
                    "fetal error: Init FCT Node Intergration faild, error code:"
                    + std::to_string(result->exit_code()));
            }
            g_initializationResultl = result;
            std::unique_ptr<node::MultiIsolatePlatform> platform =
                node::MultiIsolatePlatform::Create(GetOptimalNodeThreadPoolSize());
            v8::V8::InitializePlatform(platform.get());
            v8::V8::Initialize();
            g_platform = std::move(platform);
        }
        static void term()
        {
            v8::V8::Dispose();
            v8::V8::DisposePlatform();
            node::TearDownOncePerProcess();
        }
        void init()
        {

        }

    };

} // FCT

#endif //NODECOMMON_H
