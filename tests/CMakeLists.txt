set(CMAKE_CXX_STANDARD 20)
enable_testing()

# Helper function to configure test targets
function(configure_fct_node_target target_name)
    target_link_libraries(${target_name} PUBLIC FCT_Node GTest::gtest GTest::gtest_main)
    set_utf8_for_target(${target_name})
    
    if(WIN32)
        # Copy node.dll (or libnode.dll) from the source build output to the test executable directory
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NODE_DLL}"
            $<TARGET_FILE_DIR:${target_name}>
        )
    endif()
endfunction()

find_package(GTest CONFIG REQUIRED)

add_executable(CustomModulePathTest CustomModulePath.cpp)
configure_fct_node_target(CustomModulePathTest)

add_executable(PromiseTest PromiseTest.cpp)
configure_fct_node_target(PromiseTest)

add_executable(StressTest StressTest.cpp)
configure_fct_node_target(StressTest)

add_executable(InteropTest InteropTest.cpp)
configure_fct_node_target(InteropTest)

add_executable(ExtendedTests ExtendedTests.cpp)
configure_fct_node_target(ExtendedTests)

add_test(NAME CustomModulePathTest COMMAND CustomModulePathTest)
add_test(NAME PromiseTest COMMAND PromiseTest)
add_test(NAME StressTest COMMAND StressTest)
add_test(NAME InteropTest COMMAND InteropTest)
add_test(NAME ExtendedTests COMMAND ExtendedTests)