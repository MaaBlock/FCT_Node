cmake_minimum_required(VERSION 3.10)
enable_testing()

add_executable(CusstomModulePathTest CusstomModulePath.cpp)
configure_fct_node_target(CusstomModulePathTest)

add_executable(PromiseTest PromiseTest.cpp)
configure_fct_node_target(PromiseTest)

add_test(NAME CustomModulePathTest
        COMMAND CusstomModulePathTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME PromiseTest
        COMMAND PromiseTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(PromiseTest PROPERTIES TIMEOUT 30)