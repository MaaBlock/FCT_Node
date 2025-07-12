//
// Created by Administrator on 2025/7/12.
//

#include <FCT_Node.h>
#include <iostream>
#include <chrono>
#include <thread>
using namespace FCT;
using namespace std;

int main() {
    cout << "=== Promise 等待测试 ===" << endl;

    NodeCommon::Init();

    NodeEnvironment env;

    env.addModulePath("./node_modules");

    env.setup();

    env.excuteScript(R"(
function waitOneSecond() {
    return new Promise((resolve) => {
        console.log('begin wait 1 second...');
        setTimeout(() => {
            resolve('wait finish!');
        }, 1000);
    });
}
    )");

    auto startTime = chrono::high_resolution_clock::now();

    cout << "调用Promise函数..." << endl;

    auto promise = env.callFunction<JSPromise>("waitOneSecond");

    while (!promise.isSettled()) {
        env.tick();
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

    if (promise.isFulfilled()) {
        string result = promise.getResult<string>();
        cout << "Promise结果: " << result << endl;
        cout << "实际等待时间: " << duration.count() << "ms" << endl;
    }

    env.stop();
    NodeCommon::Term();

    cout << "=== 测试完成 ===" << endl;
    return 0;
}