set(TARGET MainWindow)

set(SOURCE_FILES
    MainWindow.cpp
    WindowDX12.cpp
    CommandAllocatorPool.cpp
)

add_executable(${TARGET} ${SOURCE_FILES})

target_link_libraries(${TARGET} PUBLIC
    Utils
    SDL2-static
    SDL2main
    d3dx12

    dxgi
    d3d12
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "HelloWindow")
