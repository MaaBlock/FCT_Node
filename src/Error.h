//
// Created by Administrator on 2025/5/29.
//

#ifndef FCT_NODE_ERROR_H
#define FCT_NODE_ERROR_H
#include <stdexcept>
#include <string>
namespace FCT {
    class NodeError : public std::runtime_error {
    public:
        explicit NodeError(const std::string& message)
            : std::runtime_error("FCT_NodeError: " + message) {}
    };
}
#endif //ERROR_H
