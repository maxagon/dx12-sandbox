
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(D3D12Sandbox)

# Sources
add_subdirectory(Sources/HelloWindow)

# 3rd party
set(SDL_SHARED 0)
add_subdirectory(3rdparty/SDL)
