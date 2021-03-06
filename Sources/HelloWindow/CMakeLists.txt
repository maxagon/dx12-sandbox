set(TARGET MainWindow)

set(SOURCE_FILES
    MainWindow.cpp
)

add_executable(${TARGET} ${SOURCE_FILES})

target_link_libraries(${TARGET} PUBLIC
    SDL2-static
    SDL2main
)

#target_link_options(${TARGET} "/MD")

set_property(TARGET ${TARGET} PROPERTY FOLDER "HelloWindow")
