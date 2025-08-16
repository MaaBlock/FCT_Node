function(add_copy_target_headers_custom_src target_name headers_list src_dir)
    set(copy_target_name "copy_${target_name}_headers")

    set(include_base_dir "${CMAKE_CURRENT_SOURCE_DIR}/include/${target_name}")

    file(MAKE_DIRECTORY ${include_base_dir})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)

    set(copied_headers "")

    foreach(header_file ${headers_list})
        if(NOT IS_ABSOLUTE ${header_file})
            set(header_file "${CMAKE_CURRENT_SOURCE_DIR}/${header_file}")
        endif()

        file(RELATIVE_PATH relative_path ${src_dir} ${header_file})
        set(output_file "${include_base_dir}/${relative_path}")
        get_filename_component(output_dir "${output_file}" DIRECTORY)

        file(MAKE_DIRECTORY ${output_dir})

        add_custom_command(
                OUTPUT ${output_file}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${header_file}
                ${output_file}
                DEPENDS ${header_file}
                COMMENT "Copying: ${relative_path}"
        )

        list(APPEND copied_headers ${output_file})
    endforeach()

    set(main_header_file "${src_dir}/${target_name}.h")
    if(EXISTS ${main_header_file})
        set(main_output_file "${CMAKE_CURRENT_SOURCE_DIR}/include/${target_name}.h")
        add_custom_command(
                OUTPUT ${main_output_file}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${main_header_file}
                ${main_output_file}
                DEPENDS ${main_header_file}
                COMMENT "Copying main header: ${target_name}.h"
        )
        list(APPEND copied_headers ${main_output_file})
    endif()

    add_custom_target(${copy_target_name} ALL
            DEPENDS ${copied_headers}
            COMMENT "Headers for ${target_name} are up to date"
    )

    if(TARGET ${target_name})
        add_dependencies(${target_name} ${copy_target_name})
    endif()

    message(STATUS "Added copy headers target for: ${target_name}")
endfunction()

function(add_copy_target_headers target_name headers_list)
    add_copy_target_headers_custom_src(${target_name} "${headers_list}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
endfunction()