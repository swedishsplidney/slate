cmake_minimum_required(VERSION 3.22)
project(slate VERSION 0.1.0 LANGUAGES CXX)

# force c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# collect source files (recursively)
file(GLOB_RECURSE ENGINE_SOURCES
    "src/*.cpp"
    "src/*.hpp"
)

# define the executable
add_executable(slate ${ENGINE_SOURCES}
        src/renderer/vulkan/vulkan_renderer.hpp
        src/renderer/vulkan/vulkan_renderer.cpp)

# include dirs
target_include_directories(slate PRIVATE
    src
    ${Vulkan_INCLUDE_DIRS}
)

# link libraries
target_link_libraries(slate PRIVATE
    Vulkan::Vulkan
    glfw
)

message(STATUS "slate engine configured successfully!")