cmake_minimum_required(VERSION 3.10)
project(FCT_Node)
include("config/copy_headers.cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(node)

file(GLOB FCT_NODE_HEADERS
        "src/*.h"
        "src/*.hpp"
)

list(REMOVE_ITEM FCT_NODE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_Node.h")

add_library(FCT_Node STATIC
        src/headers.cpp
        src/NodeCommon.cpp
        src/NodeEnvironment.cpp
        src/JSObject.cpp
        src/JSPromise.cpp
        src/ExplicitMicrotasksScope.cpp
        src/FunctionWrapper.cpp
)

target_include_directories(FCT_Node PUBLIC
        ${NODE_INCLUDE_DIR}
        ${V8_INCLUDE_DIR}
        ${UV_INCLUDE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(FCT_Node PUBLIC
        nodejs_deps
)

add_copy_target_headers(FCT_Node "${FCT_NODE_HEADERS}")

if(DEFINED NODE_LIB_DIR)
    set(NODE_LIB_DIR ${NODE_LIB_DIR} PARENT_SCOPE)
endif()

add_custom_target(export_all ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/FCT_Node/Include/FCT_Node"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/FCT_Node/Lib"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/FCT_Node/Bin"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/V8_Headers/Include"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/UV_Headers/Include"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/Node_Headers/Include"
        COMMENT "Creating export directory structure"
)

if(FCT_NODE_HEADERS)
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FCT_NODE_HEADERS}
            "${CMAKE_BINARY_DIR}/export/FCT_Node/Include/FCT_Node/"
            COMMENT "Copying FCT_Node headers to export/FCT_Node/Include"
            VERBATIM
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_Node.h")
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_Node.h"
            "${CMAKE_BINARY_DIR}/export/FCT_Node/Include/"
            COMMENT "Copying FCT_Node.h to export root directory"
            VERBATIM
    )
endif()


add_custom_command(TARGET export_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:FCT_Node>"
        "${CMAKE_BINARY_DIR}/export/FCT_Node/Lib/"
        COMMENT "Copying FCT_Node library to export/FCT_Node/Lib"
)

if(DEFINED NODE_INCLUDE_DIR AND EXISTS "${NODE_INCLUDE_DIR}")
    file(GLOB_RECURSE NODE_HEADER_FILES
            "${NODE_INCLUDE_DIR}/*.h"
            "${NODE_INCLUDE_DIR}/*.hpp"
            "${NODE_INCLUDE_DIR}/*.hxx"
            "${NODE_INCLUDE_DIR}/*.inl"
    )

    if(NODE_HEADER_FILES)
        add_custom_command(TARGET export_all POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/Node_Headers/Include"
                COMMENT "Creating Node_Headers/Include directory"
        )

        foreach(HEADER_FILE ${NODE_HEADER_FILES})
            file(RELATIVE_PATH REL_PATH "${NODE_INCLUDE_DIR}" "${HEADER_FILE}")
            get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)

            add_custom_command(TARGET export_all POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/export/Node_Headers/Include/${REL_DIR}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${HEADER_FILE}"
                    "${CMAKE_BINARY_DIR}/export/Node_Headers/Include/${REL_PATH}"
                    COMMENT "Copying Node header: ${REL_PATH}"
            )
        endforeach()
    endif()
endif()

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

if(DEFINED NODE_LIB_DIR AND EXISTS "${NODE_LIB_DIR}/libnode.lib")
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NODE_LIB_DIR}/libnode.lib"
            "${CMAKE_BINARY_DIR}/export/FCT_Node/Lib/"
            COMMENT "Copying libnode.lib to export/FCT_Node/Lib"
    )
endif()

if(DEFINED NODE_DEPS_LIB_DIR AND EXISTS "${NODE_DEPS_LIB_DIR}")
    add_custom_command(TARGET export_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${NODE_DEPS_LIB_DIR}"
            "${CMAKE_BINARY_DIR}/export/FCT_Node/Lib/"
            COMMENT "Copying Node dependencies libraries to export/FCT_Node/Lib"
    )
endif()

add_dependencies(export_all FCT_Node)

message(STATUS "Export will be created at: ${CMAKE_BINARY_DIR}/export")
message(STATUS "V8_INCLUDE_DIR: ${V8_INCLUDE_DIR}")
message(STATUS "UV_INCLUDE_DIR: ${UV_INCLUDE_DIR}")
message(STATUS "NODE_LIB_DIR: ${NODE_LIB_DIR}")
message(STATUS "NODE_DEPS_LIB_DIR: ${NODE_DEPS_LIB_DIR}")

function(configure_fct_node_target target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist")
    endif()

    target_link_libraries(${target_name} PUBLIC FCT_Node)

    if(DEFINED NODE_LIB_DIR)
        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${NODE_LIB_DIR}/libnode.dll"
                $<TARGET_FILE_DIR:${target_name}>
                COMMENT "Copying libnode.dll for ${target_name}"
        )
    endif()

    message(STATUS "Configured FCT_Node for target: ${target_name}")
endfunction()

function(target_link_fct_node_libraries target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist")
    endif()

    target_link_libraries(${target_name} FCT_Node)

    if(DEFINED NODE_LIB_DIR)
        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${NODE_LIB_DIR}/libnode.dll"
                $<TARGET_FILE_DIR:${target_name}>
                COMMENT "Copying libnode.dll for ${target_name}"
        )
    endif()

    message(STATUS "Configured FCT_Node for target: ${target_name}")
endfunction()

add_subdirectory(tests)

