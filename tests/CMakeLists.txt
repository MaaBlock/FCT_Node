cmake_minimum_required(VERSION 3.10)
enable_testing()

add_executable(CusstomModulePathTest CusstomModulePath.cpp)
configure_fct_node_target(CusstomModulePathTest)

add_test(NAME CustomModulePathTest
        COMMAND CusstomModulePathTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
