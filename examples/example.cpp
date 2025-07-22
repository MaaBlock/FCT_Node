/**
 * @example
 */

#include <FCT_Node.h>
using namespace FCT;
using namespace std;
//所有均在FCT命名空间下
int main() {
    // 初始化Node.js环境
    NodeCommon::Init();
    // 创建Node.js环境
    NodeEnvironment env;

    // 添加npm包搜索路径
    env.addModulePath("./node_modules");
    //不添加也可，默认会搜索工作目录所在的node_modules
    env.addModulePath("F:/node_modules");
    // 自定义模块路径

    env.setup();//启动
    // 示例：基础JavaScript函数调用
    env.excuteScript(R"(
    function foo(){ //在后续被调用
        console.log('hello world!');
        return 'hello from js';
    }

    console.log('hello')//直接被当场执行
    )");
    cout << env.callFunction<std::string>("foo") << endl;
    // callFunction<返回值类型>(函数名，函数参数0，函数参数1,....)
    //示例 c++回调
    env.excuteScript("var cppAdd;");
    env.global()["cppAdd"] = [](int a, int b) {
        return a + b;
    };
    env.excuteScript(R"(
        console.log('Testing C++ function from JS:');
        console.log('cppAdd(5, 3) =', cppAdd(5, 3));
)");
    // 示例：JSObject操作示例
    env.excuteScript(R"(
    function createUser(name, age) {
        return {
            name: name,
            age: age,
            email: name.toLowerCase() + '@example.com',
            profile: {
                level: 1,
                score: 100
            },
            greet: function() {
                return 'Hello, I am ' + this.name;
            }
        };
    }

    function updateUserScore(user, newScore) {
        user.profile.score = newScore;
        return user;
    }
    )");

    cout << "\n=== JSObject 示例 ===" << endl;
    {
        JSObject user = env.callFunction<JSObject>("createUser", "Alice", 25);

        // 读取对象属性
        string name = user["name"];
        int age = user["age"];
        string email = user["email"];

        cout << "用户名: " << name << endl;
        cout << "年龄: " << age << endl;
        cout << "邮箱: " << email << endl;

        // 访问嵌套对象
        JSObject profile = user["profile"];
        int score = profile["score"];
        cout << "积分: " << score << endl;

        // 修改对象属性
        user["age"] = 26;
        profile["score"] = 150;

        cout << "修改后年龄: " << user.get<int>("age") << endl;
        cout << "修改后积分: " << profile.get<int>("score") << endl;
    }

    // 示例：JSPromise异步操作示例
    env.excuteScript(R"(
    function delayedMessage(message, delay) {
        return new Promise((resolve, reject) => {
            setTimeout(() => {
                if (message && message.length > 0) {
                    resolve('延迟消息: ' + message);
                } else {
                    reject('消息不能为空');
                }
            }, delay);
        });
    }

    function fetchUserData(userId) {
        return new Promise((resolve) => {
            setTimeout(() => {
                resolve({
                    id: userId,
                    name: 'User' + userId,
                    status: 'active',
                    lastLogin: new Date().toISOString()
                });
            }, 500);
        });
    }
    )");

    cout << "\n=== JSPromise 示例 ===" << endl;
    {
        cout << "发送延迟消息..." << endl;
        auto promise = env.callFunction<JSPromise>("delayedMessage", "Hello FCT!", 1000);

        while (!promise.isSettled())
        {
            env.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (promise.isFulfilled())
        {
            string result = promise.getResult<string>();
            cout << "Promise结果: " << result << endl;
        }
        else
        {
            cout << "Promise失败: " << promise.getError() << endl;
        }
    }

    {
        cout << "\n获取用户数据..." << endl;
        auto userPromise = env.callFunction<JSPromise>("fetchUserData", 123);

        while (!userPromise.isSettled())
        {
            env.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (userPromise.isFulfilled())
        {
            JSObject userData = userPromise.getResult<JSObject>();
            cout << "用户ID: " << userData.get<int>("id") << endl;
            cout << "用户名: " << userData.get<string>("name") << endl;
            cout << "状态: " << userData.get<string>("status") << endl;
            cout << "最后登录: " << userData.get<string>("lastLogin") << endl;
        }
        else
        {
            cout << "获取用户数据失败: " << userPromise.getError() << endl;
        }
    }
    env.stop();//暂停
    NodeCommon::Term();//结束
    return 0;
}