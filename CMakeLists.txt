cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "Error: VCPKG_ROOT environment variable is not defined. Please set it to your vcpkg installation root.")
endif()

file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" VCPKG_ROOT_PATH)
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT_PATH}/scripts/buildsystems/vcpkg.cmake")

project(NexusScript)

include("config/copy_headers.cmake")
include("config/set_utf8_compiler_encoding.cmake")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_C_SHOWINCLUDES_PREFIX "Note: including file: ")
    set(CMAKE_CXX_SHOWINCLUDES_PREFIX "Note: including file: ")
    add_compile_options(/Zc:__cplusplus)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Node.js Source Build via FetchContent ---
include(FetchContent)
FetchContent_Declare(
    nodejs
    GIT_REPOSITORY https://github.com/nodejs/node.git
    GIT_TAG        v24.13.1
    GIT_PROGRESS   TRUE
    SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/node"
)

# Populate first to get source directory
FetchContent_GetProperties(nodejs)
if(NOT nodejs_POPULATED)
    message(STATUS "Fetching Node.js v24.13.1...")
    FetchContent_Populate(nodejs)
endif()

set(NODE_SOURCE_DIR "${nodejs_SOURCE_DIR}")
set(NODE_BINARY_DIR "${nodejs_BINARY_DIR}") # Not strictly used as we build in source for Node

# Define expected outputs
if(WIN32)
    # Determine build type and set arguments/paths accordingly
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(NODE_BUILD_ARGS dll debug)
        set(NODE_OUTPUT_DIR "${NODE_SOURCE_DIR}/out/Debug")
    else()
        set(NODE_BUILD_ARGS dll release)
        set(NODE_OUTPUT_DIR "${NODE_SOURCE_DIR}/out/Release")
    endif()

    set(NODE_LIB "${NODE_OUTPUT_DIR}/libnode.lib")
    set(NODE_DLL "${NODE_OUTPUT_DIR}/libnode.dll")

    # Environment Handling for Windows
    # We use vswhere.exe to find the Visual Studio installation path
    find_program(VSWHERE_EXE vswhere.exe
        HINTS
        "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
        "$ENV{ProgramFiles}/Microsoft Visual Studio/Installer"
    )

    if(VSWHERE_EXE)
        execute_process(
            COMMAND "${VSWHERE_EXE}" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            OUTPUT_VARIABLE VS_INSTALL_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(VS_INSTALL_PATH)
            file(TO_CMAKE_PATH "${VS_INSTALL_PATH}" VS_INSTALL_PATH)
            message(STATUS "Found Visual Studio at: ${VS_INSTALL_PATH}")
            set(VCVARSALL_BAT "${VS_INSTALL_PATH}/VC/Auxiliary/Build/vcvarsall.bat")
        endif()
    endif()

    if(EXISTS "${VCVARSALL_BAT}")
        # If found, wrap execution in environment setup
        # Note: vcvarsall.bat changes cwd, so we push/pop or use &&
        # We also check if the library already exists to avoid re-running the expensive build
        
        file(TO_NATIVE_PATH "${VCVARSALL_BAT}" VCVARSALL_BAT_NATIVE)
        file(TO_NATIVE_PATH "${VCVARSALL_BAT}" VCVARSALL_BAT_NATIVE)
        file(TO_NATIVE_PATH "${NODE_LIB}" NODE_LIB_NATIVE)
        file(TO_NATIVE_PATH "${NODE_DLL}" NODE_DLL_NATIVE)

        # Windows command line handling for nested quotes is terrible.
        # We construct a command that checks existence and then calls the build script.
        # Check both LIB and DLL. If either is missing, we must build.
        # Logic: if exist LIB (if exist DLL (skip) else (BUILD)) else (BUILD)
        # Note: We rely on the parent environment (build.bat) to have set up MSVC environment.
        set(BUILD_CMD 
            cmd /c if exist "${NODE_LIB_NATIVE}" (
                if exist "${NODE_DLL_NATIVE}" (
                    echo "Node.js library found, skipping build"
                ) else (
                    call vcbuild.bat ${NODE_BUILD_ARGS}
                )
            ) else (
                call vcbuild.bat ${NODE_BUILD_ARGS}
            )
        )
    else()
        message(WARNING "vcvarsall.bat not found. Trying direct execution.")
        # Fallback to direct execution, hoping user has environment
        file(TO_NATIVE_PATH "${NODE_LIB}" NODE_LIB_NATIVE)
        set(BUILD_CMD cmd /c "if not exist ${NODE_LIB_NATIVE} (vcbuild.bat ${NODE_BUILD_ARGS}) else (echo Node.js library found, skipping build)")
    endif()
    
else()
    set(NODE_LIB "${NODE_SOURCE_DIR}/out/Release/libnode.so")
    set(NODE_DLL "${NODE_LIB}")
    
    include(ProcessorCount)
    ProcessorCount(N)
    
    # Linux build command
    set(BUILD_CMD /bin/bash -c "./configure --shared && make -j${N}")
endif()

# Define Custom Build Target
# We use a stamp file to avoid rebuilding every time
set(NODE_BUILD_STAMP "${CMAKE_CURRENT_BINARY_DIR}/node_build.stamp")

# Define outputs to avoid duplicate rules on Linux where libnode.so is both LIB and DLL
set(NODE_ALL_OUTPUTS ${NODE_BUILD_STAMP} ${NODE_LIB})
if(NOT NODE_LIB STREQUAL NODE_DLL)
    list(APPEND NODE_ALL_OUTPUTS ${NODE_DLL})
endif()

add_custom_command(
    OUTPUT ${NODE_ALL_OUTPUTS}
    COMMAND ${BUILD_CMD}
    COMMAND ${CMAKE_COMMAND} -E touch ${NODE_BUILD_STAMP}
    WORKING_DIRECTORY ${NODE_SOURCE_DIR}
    COMMENT "Building Node.js from source (${NODE_SOURCE_DIR})..."
    VERBATIM
    USES_TERMINAL
)

# Make the custom target depend on ALL outputs to ensure they exist for downstream consumers
add_custom_target(build_nodejs DEPENDS ${NODE_ALL_OUTPUTS})

# Define an IMPORTED target to allow downstream targets in other directories to see the dependency
add_library(libnode_imported SHARED IMPORTED GLOBAL)
add_dependencies(libnode_imported build_nodejs)

set_target_properties(libnode_imported PROPERTIES
    IMPORTED_LOCATION "${NODE_DLL}"
)
if(WIN32)
    set_target_properties(libnode_imported PROPERTIES
        IMPORTED_IMPLIB "${NODE_LIB}"
    )
endif()

# Interface library for consumers
add_library(nodejs_deps INTERFACE)
target_include_directories(nodejs_deps INTERFACE
    ${NODE_SOURCE_DIR}/src
    ${NODE_SOURCE_DIR}/deps/v8/include
    ${NODE_SOURCE_DIR}/deps/uv/include
)

# Link against the imported target to ensure build ordering and visibility
target_link_libraries(nodejs_deps INTERFACE libnode_imported)

# Add required macros for V8 shared library usage
target_compile_definitions(nodejs_deps INTERFACE)

if(WIN32)
    target_compile_definitions(nodejs_deps INTERFACE USING_V8_SHARED=1 BUILDING_NODE_EXTENSION=1)
endif()

add_dependencies(nodejs_deps build_nodejs)


# --- FCT_Node Library ---

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
        src/JSAny.cpp
        src/JSPromise.cpp
        src/ExplicitMicrotasksScope.cpp
        src/FunctionWrapper.cpp
)

target_include_directories(FCT_Node PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(FCT_Node PUBLIC nodejs_deps)
set_utf8_for_target(FCT_Node)

if(MSVC)
    target_compile_options(FCT_Node PRIVATE /wd4251 /wd4275)
endif()

add_copy_target_headers(FCT_Node "${FCT_NODE_HEADERS}")

# Copy DLL for tests/executables
if(WIN32)
    add_custom_command(TARGET FCT_Node POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:libnode_imported>
        $<TARGET_FILE_DIR:FCT_Node>
    )
endif()

# Tests
add_subdirectory(tests)
