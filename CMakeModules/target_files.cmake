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
        if(IS_ABSOLUTE ${copy_file_src})
            get_filename_component(copy_file ${copy_file_src} NAME)
            set(copy_file_input ${copy_file_src})
            set(copy_file_out "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/${copy_file}")
        else()
            set(copy_file_input "${CMAKE_CURRENT_SOURCE_DIR}/${copy_file_src}")
            set(copy_file_out "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/${copy_file_src}")
        endif()
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
