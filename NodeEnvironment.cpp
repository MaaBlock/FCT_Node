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

    bool NodeEnvironment::setup()
    {
        auto platform = NodeCommon::GetPlatform().get();
        auto args = m_args;
        auto exec_args = m_excuteArgs;

        for (auto &arg : args)
        {
            std::cout << "Args: " << arg << std::endl;
        }
        for (auto &arg : exec_args)
        {
            std::cout << "Exec Args: " << arg << std::endl;
        }

        std::vector<std::string> errors;
        m_setup = node::CommonEnvironmentSetup::Create(platform, &errors, args, exec_args);
        if (!m_setup) {
            for (const std::string& err : errors)
                fprintf(stderr, "%s: %s\n", args[0].c_str(), err.c_str());
            return false;
        }

        m_isolate = m_setup->isolate();
        m_env = m_setup->env();

        return true;
    }

    bool NodeEnvironment::executeArg()
    {
        return execute("require('node:vm').runInThisContext(process.argv[1]);");

        /*
    if (!m_setup || !m_isolate || !m_env) {
        std::cerr << "Environment not properly set up. Call setup() first." << std::endl;
        return false;
    }

    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Context::Scope context_scope(m_setup->context());

    std::string setupModulePaths = "";
    for (const auto& path : m_modulePaths) {
        setupModulePaths += "module.paths.push('" + path + "');\n";
    }

    v8::MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(
        m_env,
        "const publicRequire ="
        "  require('node:module').createRequire(process.cwd() + '/');"
        "globalThis.require = publicRequire;"
        + setupModulePaths +
        "require('node:vm').runInThisContext(process.argv[1]);");

    if (loadenv_ret.IsEmpty())
        return false;

    m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);
    return true;
    */
}

    /*
bool NodeEnvironment::execute(std::string_view jsCode)
{
    if (!m_setup || !m_isolate || !m_env) {
        if (!setup()) {
            return false;
        }
    }

    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Context::Scope context_scope(m_setup->context());

    std::string setupModulePaths = "";
    for (const auto& path : m_modulePaths) {
        setupModulePaths += "module.paths.push('" + path + "');\n";
    }

    std::string fullJsCode =
        "const publicRequire = require('node:module').createRequire(process.cwd() + '/');"
        "globalThis.require = publicRequire;"
        + setupModulePaths
        + std::string(jsCode);

    v8::MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(m_env, fullJsCode);

    if (loadenv_ret.IsEmpty())
        return false;

    m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);
    return true;
} */
    /*
    bool NodeEnvironment::execute(std::string_view jsCode)
    {
        if (!m_setup || !m_isolate || !m_env) {
            if (!setup()) {
                return false;
            }
        }

        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        v8::Context::Scope context_scope(m_setup->context());

        std::string setupModulePaths = "";
        for (const auto& path : m_modulePaths) {
            setupModulePaths += "module.paths.push(`" + path + "`);\n";
        }

        std::string bootstrapCode = R"(
const publicRequire = require('node:module').createRequire(process.cwd() + '/');
globalThis.require = publicRequire;

)" + setupModulePaths + R"(

const vm = require('node:vm');

const userCode = `)" + std::string(jsCode) + R"(`;

vm.runInThisContext(userCode, { filename: 'user_script.js' });
)";

        v8::MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(m_env, bootstrapCode);

        if (loadenv_ret.IsEmpty()) {
            std::cerr << "Failed to execute JavaScript code" << std::endl;
            return false;
        }

        m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);
        return true;
    }
    */
    bool NodeEnvironment::execute(std::string_view jsCode)
{
    if (!m_setup || !m_isolate || !m_env) {
        if (!setup()) {
            return false;
        }
    }

    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Context::Scope context_scope(m_setup->context());

    if (!m_environmentInitialized) {
        std::string setupModulePaths = "";
        for (const auto& path : m_modulePaths) {
            setupModulePaths += "module.paths.push('" + path + "');\n";
        }

        std::string initCode = R"(
// 设置全局 require
const publicRequire = require('node:module').createRequire(process.cwd() + '/');
globalThis.require = publicRequire;

)" + setupModulePaths + R"(

globalThis.__FCT_executeScript = function(codeBase64) {
    const vm = require('node:vm');
    try {
        const code = Buffer.from(codeBase64, 'base64').toString('utf8');
        return vm.runInThisContext(code, {
            filename: 'user_script.js',
            displayErrors: true
        });
    } catch (error) {
        console.error('Error executing script:', error);
        return { error: error.message };
    }
};

console.log('FCT Node.js environment initialized');
)";

        v8::MaybeLocal<v8::Value> init_ret = node::LoadEnvironment(m_env, initCode);

        if (init_ret.IsEmpty()) {
            std::cerr << "Failed to initialize JavaScript environment" << std::endl;
            return false;
        }

        m_environmentInitialized = true;
    }

    std::string jsCodeStr(jsCode);
    std::string base64Code = base64Encode(jsCodeStr);

    std::string executeCode = "globalThis.__FCT_executeScript('" + base64Code + "');";

    v8::MaybeLocal<v8::Value> exec_ret = node::LoadEnvironment(m_env, executeCode);

    if (exec_ret.IsEmpty()) {
        std::cerr << "Failed to execute JavaScript code" << std::endl;
        return false;
    }

    m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);

    return true;
}

std::string NodeEnvironment::base64Encode(const std::string& input) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (char c : input) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}
void NodeEnvironment::callFunction(const std::string& funcName, const std::vector<v8::Local<v8::Value>>& args)
{
    if (!m_setup || !m_isolate || !m_env) {
        std::cerr << "Environment not properly set up. Call setup() first." << std::endl;
        return;
    }

    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context = m_setup->context();
    v8::Context::Scope context_scope(context);

    v8::Local<v8::Object> global = context->Global();

    v8::Local<v8::String> func_name = v8::String::NewFromUtf8(m_isolate, funcName.c_str(),
                                                             v8::NewStringType::kNormal).ToLocalChecked();
    v8::MaybeLocal<v8::Value> maybe_func = global->Get(context, func_name);

    if (maybe_func.IsEmpty()) {
        std::cerr << "Function '" << funcName << "' not found in global scope" << std::endl;
        return;
    }

    v8::Local<v8::Value> func_val = maybe_func.ToLocalChecked();
    if (!func_val->IsFunction()) {
        std::cerr << "'" << funcName << "' is not a function" << std::endl;
        return;
    }

    v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(func_val);

    std::vector<v8::Local<v8::Value>> argv = args;

    node::async_context asyncContext = { 0, 0 };
    v8::MaybeLocal<v8::Value> result = node::MakeCallback(
        m_isolate,
        global,
        func,
        static_cast<int>(argv.size()),
        argv.empty() ? nullptr : argv.data(),
        asyncContext);

    if (result.IsEmpty()) {
        std::cerr << "Error calling function '" << funcName << "'" << std::endl;
        return;
    }

    m_exitCode = node::SpinEventLoop(m_env).FromMaybe(1);
}

void NodeEnvironment::callFunction(const std::string& funcName)
{
    std::vector<v8::Local<v8::Value>> args;
    callFunction(funcName, args);
}

    int NodeEnvironment::excute()
{
    if (!setup()) {
        return 1;
    }

    if (!executeArg()) {
        return 1;
    }

    return m_exitCode;
}

void NodeEnvironment::stop()
{
    if (!m_env) {
        return;
    }

    node::Stop(m_env);
}
} // FCT