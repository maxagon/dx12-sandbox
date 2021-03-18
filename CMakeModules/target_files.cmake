function(target_files target)
    cmake_parse_arguments(
        T_FILES
        ""
        ""
        "PUBLIC;INTERFACE;PRIVATE;ASSETS"
        "SOURCES"
        ${ARGN}
    )
    foreach(copy_file_src ${T_FILES_ASSETS})
        set(copy_file_out "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${copy_file_src}")
        set(copy_file_input "${CMAKE_CURRENT_SOURCE_DIR}/${copy_file_src}")
        message(${copy_file_input})
        message(${copy_file_out})
        list(APPEND copy_file_inputs ${copy_file_input})
        add_custom_command(
            OUTPUT ${copy_file_out}
            COMMAND ${CMAKE_COMMAND} -E copy ${copy_file_input} ${copy_file_out}
            MAIN_DEPENDENCY ${copy_file_input}
        )
    endforeach()

    set_property(SOURCE ${copy_file_inputs} PROPERTY GENERATED)

    target_sources(${target}
        PUBLIC
            ${SOURCES}
            ${T_FILES_PUBLIC}
        PRIVATE
            ${copy_file_inputs}
            ${T_FILES_PRIVATE}
        INTERFACE
            ${T_FILES_INTERFACE}
    )
endfunction()
