set(TARGET MainWindow)

set(SOURCE_FILES
    MainWindow.cpp
)

add_executable(${TARGET} ${SOURCE_FILES})

target_link_libraries(${TARGET} PUBLIC
    Utils
    SDL2-static
    SDL2main
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "HelloWindow")
