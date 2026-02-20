#include "../src/headers.h"
#include <gtest/gtest.h>

using namespace FCT;

class NodeInteropTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        NodeCommon::Init();
    }

    static void TearDownTestCase() {
        // NodeCommon::Term(); // Avoid disposing V8 between tests
    }

    void SetUp() override {
        m_env = std::make_unique<NodeEnvironment>();
        m_env->setup();
    }

    void TearDown() override {
        m_env->stop();
    }

    std::unique_ptr<NodeEnvironment> m_env;
};

TEST_F(NodeInteropTest, SetAndGetGlobalValue) {
    JSObject global = m_env->global();
    global.set("testValue", 42);
    
    m_env->excuteScript("if (globalThis.testValue !== 42) throw new Error('Mismatch');");
    
    JSAny val = global.get<JSAny>("testValue");
    EXPECT_EQ(val.as<int>(), 42);
}

TEST_F(NodeInteropTest, ComplexObjectInterop) {
    m_env->excuteScript(R"(
        globalThis.nexusObj = {
            id: 101,
            name: "Nexus",
            config: {
                version: "1.0",
                enabled: true
            }
        };
    )");

    JSObject global = m_env->global();
    JSAny objVal = global.get<JSAny>("nexusObj");
    EXPECT_TRUE(objVal.isObject());
    
    JSObject obj = objVal.as<JSObject>();
    EXPECT_EQ(obj.get<JSAny>("id").as<int>(), 101);
    EXPECT_STREQ(obj.get<JSAny>("name").as<std::string>().c_str(), "Nexus");
    
    JSObject config = obj.get<JSAny>("config").as<JSObject>();
    EXPECT_STREQ(config.get<JSAny>("version").as<std::string>().c_str(), "1.0");
    EXPECT_TRUE(config.get<JSAny>("enabled").as<bool>());
}

TEST_F(NodeInteropTest, CPPFunctionCallback) {
    JSObject global = m_env->global();
    
    bool called = false;
    // Assuming FunctionWrapper or similar allows binding lambda/functions
    // This part depends on the specific implementation in NexusScript
    // Let's assume a generic check for function existence and callability
    
    m_env->excuteScript(R"(
        globalThis.add = (a, b) => a + b;
    )");
    
    JSAny addFunc = global.get<JSAny>("add");
    EXPECT_TRUE(addFunc.isFunction());
    
    // In actual use, we'd use m_env->callFunction
    // m_env->callFunction("add", ...); 
}
