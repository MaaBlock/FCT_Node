/**
* @file NodeCommon.h
 * @brief Node.js common utilities and initialization management
 * @details This file provides the core Node.js initialization and termination functionality.
 *          Users should not include this file directly, but rather include FCT_Node.h instead.
 * @author FCT Team
 */

#ifndef NODECOMMON_H
#define NODECOMMON_H
#include <filesystem>


#include "./NodeEnvironment.h"

namespace FCT {
    template<typename T>
    class MouduleManager
    {
    public:
        MouduleManager()
        {
            std::cout << "MouduleManager Init" << std::endl;
            T::Init();
        }
        ~MouduleManager()
        {
            std::cout << "MouduleManager Term" << std::endl;
            T::Term();
        }
    };
    /**
  * @brief Node.js common operations and global state management
  * @details Provides static methods for initializing and terminating the Node.js environment.
  *          Must be called before using any Node.js functionality.
  */
    class NodeCommon
    {
    private:
        static std::shared_ptr<node::InitializationResult> g_initializationResultl;
        static std::unique_ptr<node::MultiIsolatePlatform> g_platform;
    public:
        /**
         * @brief Get the Node.js initialization result
         * @return Shared pointer to the initialization result
         */
        static std::shared_ptr<node::InitializationResult> GetInitializationResult() {
            return g_initializationResultl;
        }
        /**
       * @brief Get the Node.js platform instance
       * @return Reference to the platform unique pointer
       */
        static const std::unique_ptr<node::MultiIsolatePlatform>& GetPlatform()
        {
            return g_platform;
        }
        /**
        * @brief Initialize Node.js environment
        * @details Must be called once before using any Node.js functionality.
        *          Initializes V8 platform and Node.js process.
        * @throws std::runtime_error if initialization fails
        */

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

            printf("");
        }
        /*
        static void Init()
        {
            auto args = std::vector<std::string>();
            args.push_back("FctNodeApp");

            node::InitializeOncePerProcess(args);

            g_platform = node::MultiIsolatePlatform::Create(GetOptimalNodeThreadPoolSize());
        }*/
        /**
        * @brief Terminate Node.js environment
        * @details Must be called once at program exit to clean up resources.
        *          Disposes V8 platform and tears down Node.js process.
        */
        static void Term()
        {
            v8::V8::Dispose();
            v8::V8::DisposePlatform();
            node::TearDownOncePerProcess();
        }
        /**
         * @brief Instance initialization method (currently unused)
         * @deprecated Use static Init() method instead
         */
        void init()
        {

        }
    };
    //extern MouduleManager<NodeCommon> _nodeCommonManager;
} // FCT

#endif //NODECOMMON_H
