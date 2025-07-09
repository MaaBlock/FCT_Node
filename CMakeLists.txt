cmake_minimum_required(VERSION 3.10)
project(FCT_Node)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(node)

add_library(FCT_Node STATIC
        headers.cpp
        NodeCommon.cpp
        NodeEnvironment.cpp
        JSObject.cpp
        JSPromise.cpp
        ExplicitMicrotasksScope.cpp
)

target_include_directories(FCT_Node PUBLIC
        ${NODE_INCLUDE_DIR}
        ${V8_INCLUDE_DIR}
        ${UV_INCLUDE_DIR}
)

target_link_libraries(FCT_Node PUBLIC
        nodejs_deps
)

if(DEFINED NODE_LIB_DIR)
    set(NODE_LIB_DIR ${NODE_LIB_DIR} PARENT_SCOPE)
endif()