set(TARGET MainWindow)

set(SOURCE_FILES
    MainWindow.cpp
    WindowDX12.cpp
    CommandAllocatorPool.cpp
)

set(SHADER_FILES
    shaders/TestShader.hlsl
)

add_executable(${TARGET} "")

target_files(${TARGET}
    PUBLIC
        ${SOURCE_FILES}
    ASSETS
        ${SHADER_FILES}
)

target_link_libraries(${TARGET} PUBLIC
    Utils
    ShaderCompiler
    RenderingCore

    SDL2-static
    SDL2main
    d3dx12
    dxgi
    d3d12
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "HelloWindow")
