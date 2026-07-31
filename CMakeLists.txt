cmake_minimum_required(VERSION 3.22)
project(slate VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

file(GLOB_RECURSE SHADER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/compiled")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

add_executable(slate
        src/main.cpp
        src/core/engine.cpp
        src/core/engine.hpp
        src/core/camera.cpp
        src/core/camera.hpp
        src/resources/mesh_loader.cpp
        src/resources/mesh_loader.hpp
        src/renderer/renderer.hpp
        src/renderer/vulkan/vulkan_renderer.cpp
        src/renderer/vulkan/vulkan_renderer.hpp
        src/renderer/vertex.hpp
        src/renderer/mesh.hpp
        src/renderer/mesh.cpp
        src/ui/ui_element.cpp
        src/ui/ui_element.hpp
        src/ui/ui_manager.hpp
        src/ui/ui_button.hpp
        src/ui/ui_vertex.hpp
        src/resources/material.cpp
        src/resources/material.hpp
        src/ui/ui_manager.cpp
        src/ui/ui_dockspace.hpp
        external/stb/stb_truetype.cpp
        src/resources/font_loader.hpp
        src/resources/font_loader.cpp
        src/core/commands/command.hpp
        src/core/commands/command_registry.cpp
        src/core/commands/command_registry.hpp
        src/core/commands/command_history.cpp
        src/core/commands/command_history.hpp
        src/core/commands/file_commands.hpp
        src/core/commands/file_commands.cpp
        src/ui/ui_menu_bar.cpp
        src/ui/ui_menu_bar.hpp
)

foreach(SHADER_FILE ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(OUTPUT_SPV "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
    set(TARGET_NAME "compile_${SHADER_NAME}")

    add_custom_target(${TARGET_NAME} ALL
            COMMAND ${GLSLC_EXECUTABLE} ${SHADER_FILE} -o ${OUTPUT_SPV}
            DEPENDS ${SHADER_FILE}
            COMMENT "FORCING SHADER COMPILATION: ${SHADER_NAME} -> ${SHADER_NAME}.spv"
    )

    add_dependencies(slate ${TARGET_NAME})
endforeach()

target_include_directories(slate PRIVATE
        src
        ${Vulkan_INCLUDE_DIRS}
        external/tinyobjloader
        external/earcut
        external/stb
        external/pfd
        external/json
)

target_link_libraries(slate PRIVATE
        Vulkan::Vulkan
        SDL3::SDL3
        glm::glm
)

add_custom_command(
        TARGET slate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/models"
        "${CMAKE_CURRENT_BINARY_DIR}/models"
        COMMENT "copying assets and models to build output directory..."
)

message(STATUS "slate engine configured successfully with SDL3!")