#include <queue>

#include <io.h>
#include <FCT_Node.h>
#include <filesystem>
using namespace FCT;
using namespace std;
int main()
{
    NodeCommon::Init();
    NodeEnvironment env;
    auto current_path = std::filesystem::current_path();
    cout << "Current working directory: " << current_path << endl;

    // 检查相对路径是否存在
    std::filesystem::path relative_path = "./CusstomModulePath/node_modules";
    cout << "Checking relative path: " << relative_path << endl;
    cout << "Relative path exists: " << std::filesystem::exists(relative_path) << endl;

    // 尝试绝对路径
    std::filesystem::path absolute_path = current_path / "CusstomModulePath" / "node_modules";
    cout << "Absolute path: " << absolute_path << endl;
    cout << "Absolute path exists: " << std::filesystem::exists(absolute_path) << endl;

    // 检查tests目录下的路径
    std::filesystem::path tests_path = current_path / ".." / "tests" / "CusstomModulePath" / "node_modules";
    cout << "Tests path: " << tests_path << endl;
    cout << "Tests path exists: " << std::filesystem::exists(tests_path) << endl;

    env.addModulePath("./CusstomModulePath/node_modules");
    wcout.imbue(locale(".UTF-8"));
    env.setup();
    env.excuteScript(R"(
const Module = require('module');
Module.registerHooks({
  resolve: (specifier, context, nextResolve) => {
    console.log('Resolving', specifier);
    return nextResolve(specifier, context);
  }
});
const uuid = require('uuid')
)");
    env.stop();
    NodeCommon::Term();
    return 0;
}