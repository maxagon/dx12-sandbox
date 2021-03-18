set(TARGET DirectXShaderCompiler)

include(FetchContent)

FetchContent_Declare(
    dxcompiler
    URL        https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.5.2010/dxc_2020_10-22.zip
    SOURCE_DIR dxcompiler
    URL_HASH   MD5=BC76AB2E78DC2FA8A238EE0113F319DF
)

if(NOT dxcompiler_POPULATED)
    FetchContent_Populate(dxcompiler)
endif()

add_library(${TARGET} INTERFACE)

target_files(${TARGET}
    ASSETS
    "${dxcompiler_SOURCE_DIR}/bin/x64/dxcompiler.dll"
    "${dxcompiler_SOURCE_DIR}/bin/x64/dxil.dll"
)

target_include_directories(${TARGET} INTERFACE "${dxcompiler_SOURCE_DIR}/inc")
target_link_libraries(${TARGET} INTERFACE "${dxcompiler_SOURCE_DIR}/lib/x64/dxcompiler.lib")

set_property(TARGET ${TARGET} PROPERTY FOLDER "DirectXShaderCompiler")
