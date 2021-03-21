set(TARGET ShaderCompiler)

set(SOURCE_FILES
    ShaderCompiler.cpp
)

add_library(${TARGET} "")

target_link_libraries(${TARGET} PUBLIC
    DirectXShaderCompiler
    Utils
)

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_files(${TARGET}
    PRIVATE
        ${SOURCE_FILES}
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "ShaderCompiler")
