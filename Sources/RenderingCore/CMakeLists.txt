
set(TARGET RenderingCore)

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET} INTERFACE
    Utils
    d3dx12
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "RenderingCore")
