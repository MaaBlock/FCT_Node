//
// Created by Administrator on 2025/5/22.
//

#include "./headers.h"

namespace FCT {
 	int GetOptimalNodeThreadPoolSize(){
        unsigned int hardware_concurrency = std::thread::hardware_concurrency();

        if (hardware_concurrency == 0) {
            hardware_concurrency = 4;
        }

        return hardware_concurrency;
    }

    void NodeEnvironment::args(const std::vector<std::string> args)
    {
        m_args = args;
    }

    void NodeEnvironment::excuteArgs(const std::vector<std::string> executeArgs)
    {
        m_excuteArgs = executeArgs;
    }

    void NodeEnvironment::addModulePath(const std::string& path)
    {
        m_modulePaths.push_back(path);
    }
    int NodeEnvironment::excute()
    {
        auto platform = NodeCommon::GetPlatform().get();
        auto args = m_args;
        auto exec_args = m_excuteArgs;
        int exitCode = 0;

        for (auto &arg : args)
        {
            std::cout << "Args: " << arg << std::endl;
        }
        for (auto &arg : exec_args)
        {
            std::cout << "Exec Args: " << arg << std::endl;
        }

        std::vector<std::string> errors;
        std::unique_ptr<node::CommonEnvironmentSetup> setup =
            node::CommonEnvironmentSetup::Create(platform, &errors, args, exec_args);
        if (!setup) {
            for (const std::string& err : errors)
                fprintf(stderr, "%s: %s\n", args[0].c_str(), err.c_str());
            return 1;
        }

        v8::Isolate* isolate = setup->isolate();
        node::Environment* env = setup->env();

        {
            v8::Locker locker(isolate);
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Context::Scope context_scope(setup->context());

            std::string setupModulePaths = "";
            for (const auto& path : m_modulePaths) {
                setupModulePaths += "module.paths.push('" + path + "');\n";
            }

            v8::MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(
                env,
                "const publicRequire ="
                "  require('node:module').createRequire(process.cwd() + '/');"
                "globalThis.require = publicRequire;"
                + setupModulePaths +
                "require('node:vm').runInThisContext(process.argv[1]);");

            if (loadenv_ret.IsEmpty())
                return 1;

            exitCode = node::SpinEventLoop(env).FromMaybe(1);

            node::Stop(env);
        }

        return exitCode;
    }
} // FCT