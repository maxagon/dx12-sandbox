set(TARGET Utils)

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET} INTERFACE
    fmt
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "Utils")
