//
// Created by Administrator on 2025/5/22.
//

#ifndef NODEENVIRONMENT_H
#define NODEENVIRONMENT_H

namespace FCT {

    int GetOptimalNodeThreadPoolSize();

    class NodeEnvironment
    {
    protected:
        std::vector<std::string> m_args;
        std::vector<std::string> m_excuteArgs;
        std::vector<std::string> m_modulePaths;
    public:
        NodeEnvironment() = default;
        void args(const std::vector<std::string> args);
        void excuteArgs(const std::vector<std::string> executeArgs);
        void addModulePath(const std::string& path);
        int excute();
    };
} // FCT

#endif //NODEENVIRONMENT_H
