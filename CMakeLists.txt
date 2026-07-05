cmake_minimum_required(VERSION 3.22)
project(slate VERSION 0.1.0 LANGUAGES CXX)

# force c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find dependencies
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

# auto shader compilation stuff
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

file(GLOB_RECURSE SHADER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/compiled")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

set(COMPILED_SHADERS "")
foreach(SHADER_FILE ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(OUTPUT_SPV "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
            OUTPUT ${OUTPUT_SPV}
            COMMAND ${GLSLC_EXECUTABLE} ${SHADER_FILE} -o ${OUTPUT_SPV}
            DEPENDS ${SHADER_FILE}
            COMMENT "Compiling shader: ${SHADER_NAME} -> ${SHADER_NAME}.spv"
    )
    list(APPEND COMPILED_SHADERS ${OUTPUT_SPV})
endforeach()

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
        ${COMPILED_SHADERS}
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