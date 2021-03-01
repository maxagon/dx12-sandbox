set(TARGET MainWindow)

set(SOURCE_FILES
    MainWindow.cpp
)

add_executable(${TARGET} ${SOURCE_FILES})

set_property(TARGET ${TARGET} PROPERTY FOLDER "HelloWindow")
