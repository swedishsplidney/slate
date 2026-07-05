cmake_minimum_required(VERSION 3.22)
project(slate VERSION 0.1.0 LANGUAGES CXX)

# force c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find dependencies
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

# define the executable
add_executable(slate
        src/main.cpp
        src/core/engine.cpp
        src/core/engine.hpp
        src/renderer/renderer.hpp
        src/renderer/vulkan/vulkan_renderer.cpp
        src/renderer/vulkan/vulkan_renderer.hpp
        src/renderer/vertex.hpp
        src/renderer/mesh.hpp
        src/renderer/mesh.cpp
)

# include dirs
target_include_directories(slate PRIVATE
        src
        ${Vulkan_INCLUDE_DIRS}
)

# link libraries
target_link_libraries(slate PRIVATE
        Vulkan::Vulkan
        SDL3::SDL3
)

message(STATUS "slate engine configured successfully with SDL3!")