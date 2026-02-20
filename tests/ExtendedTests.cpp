#include "../src/headers.h"
#include <gtest/gtest.h>

using namespace FCT;

class ExtendedNodeTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        NodeCommon::Init();
    }
    static void TearDownTestCase() {
        // NodeCommon::Term();
    }
    void SetUp() override {
        m_env = std::make_unique<NodeEnvironment>();
        m_env->setup();
    }
    void TearDown() override {
        if (m_env) {
            m_env->stop();
        }
    }
    std::unique_ptr<NodeEnvironment> m_env;
};

TEST_F(ExtendedNodeTest, BasicTest) {
    m_env->excuteScript("globalThis.a = 10;");
    JSObject global = m_env->global();
    EXPECT_EQ(global.get<int>("a"), 10);
}
