cmake_minimum_required(VERSION 3.10)
project(FCT_Node)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(node)

file(GLOB FCT_NODE_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

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



# 创建导出目录结构和复制文件
add_custom_target(export_all ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/FCT_Node/Include"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/FCT_Node/Lib"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/FCT_Node/Bin"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/V8_Headers/Include"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/UV_Headers/Include"
        COMMENT "Creating export directory structure"
)

# 复制FCT_Node头文件
if(FCT_NODE_HEADERS)
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FCT_NODE_HEADERS}
            "${CMAKE_BINARY_DIR}/export/FCT_Node/Include/"
            COMMENT "Copying FCT_Node headers to export/FCT_Node/Include"
            VERBATIM
    )
endif()

add_custom_command(TARGET export_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:FCT_Node>"
        "${CMAKE_BINARY_DIR}/export/FCT_Node/Lib/"
        COMMENT "Copying FCT_Node library to export/FCT_Node/Lib"
)

if(DEFINED V8_INCLUDE_DIR AND EXISTS "${V8_INCLUDE_DIR}")
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${V8_INCLUDE_DIR}"
            "${CMAKE_BINARY_DIR}/export/V8_Headers/Include"
            COMMENT "Copying V8 headers to export/V8_Headers/Include"
    )
endif()

if(DEFINED UV_INCLUDE_DIR AND EXISTS "${UV_INCLUDE_DIR}")
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${UV_INCLUDE_DIR}"
            "${CMAKE_BINARY_DIR}/export/UV_Headers/Include"
            COMMENT "Copying UV headers to export/UV_Headers/Include"
    )
endif()

if(DEFINED NODE_LIB_DIR AND EXISTS "${NODE_LIB_DIR}/libnode.dll")
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NODE_LIB_DIR}/libnode.dll"
            "${CMAKE_BINARY_DIR}/export/FCT_Node/Bin/"
            COMMENT "Copying libnode.dll to export/FCT_Node/Bin"
    )
endif()

add_dependencies(export_all FCT_Node)

message(STATUS "Export will be created at: ${CMAKE_BINARY_DIR}/export")
message(STATUS "V8_INCLUDE_DIR: ${V8_INCLUDE_DIR}")
message(STATUS "UV_INCLUDE_DIR: ${UV_INCLUDE_DIR}")
message(STATUS "NODE_LIB_DIR: ${NODE_LIB_DIR}")