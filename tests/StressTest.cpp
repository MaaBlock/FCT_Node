#include <gtest/gtest.h>
#include "../src/headers.h"
#include <thread>
#include <vector>
#include <atomic>

using namespace FCT;

class NodeStressTest : public ::testing::Test {
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
        m_env->stop();
    }

    std::unique_ptr<NodeEnvironment> m_env;
};

TEST_F(NodeStressTest, RapidLifecycle) {
    for (int i = 0; i < 10; ++i) {
        NodeEnvironment env;
        EXPECT_TRUE(env.setup());
        env.excuteScript("var a = 1 + 1;");
        env.stop();
    }
}

TEST_F(NodeStressTest, MultiThreadedScriptExecution) {
    std::atomic<int> completed_threads{0};
    const int thread_count = 5;
    std::vector<std::thread> threads;

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, &completed_threads, i]() {
            // We need to be careful with V8 isolate thread safety
            // NodeEnvironment::excuteScript already handles v8::Locker
            std::string code = "globalThis.thread_" + std::to_string(i) + " = " + std::to_string(i) + ";";
            m_env->excuteScript(code);
            completed_threads++;
        });
    }

    for (auto& t : threads) t.join();
    EXPECT_EQ(completed_threads.load(), thread_count);

    // Verify results in main loop
    m_env->excuteScript(R"(
        if (globalThis.thread_0 !== 0 || globalThis.thread_4 !== 4) {
            throw new Error('Thread data mismatch');
        }
    )");
}

TEST_F(NodeStressTest, LargeObjectAllocation) {
    m_env->excuteScript(R"(
        globalThis.largeData = new Array(1000000).fill(0).map((_, i) => i);
        if (globalThis.largeData.length !== 1000000) throw new Error('Allocation failed');
    )");
    
    JSObject global = m_env->global();
    JSAny data = global.get<JSAny>("largeData");
    EXPECT_TRUE(data.isObject());
}
