
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(D3D12Sandbox)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

include(target_files)

# Sources
add_subdirectory(Sources/HelloWindow)
add_subdirectory(Sources/Utils)

# 3rd party
set(SDL_SHARED 0)
add_subdirectory(3rdparty/SDL)

add_subdirectory(3rdparty/d3dx12)
