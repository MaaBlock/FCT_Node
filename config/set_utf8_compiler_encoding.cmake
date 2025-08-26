function(set_utf8_for_target target_name)
    if(MSVC) # msvc
        target_compile_options(${target_name} PRIVATE
                /source-charset:utf-8    # 输入编码
                /execution-charset:utf-8 # 执行编码
        )
    else() # gcc 或 clang
        target_compile_options(${target_name} PRIVATE
                -finput-charset=UTF-8     # 输入编码
                -fexec-charset=UTF-8      # 执行编码
        )
    endif()
endfunction()