cmake_minimum_required(VERSION 3.0.0)
project(GFX-LAB VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

set(GLAD_SOURCE vendor/glad/src/gl.c)
set(IMGUI_SOURCES
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/imgui_impl/imgui_impl_glfw.cpp
        vendor/imgui/imgui_impl/imgui_impl_opengl3.cpp
        )

set(VENDOR_SOURCES ${GLAD_SOURCE} ${IMGUI_SOURCES})

#add_library(vendor_libs SHARED ${VENDOR_SOURCES})
#target_link_libraries(vendor_libs glfw)

set(COMMON_SOURCES
        source/common/application.cpp source/common/shader.cpp source/common/mesh/mesh.cpp)

include_directories(
        source/common
        vendor/glfw/include
        vendor/glad/include
        vendor/glm
        vendor/imgui
        vendor/utils
)

add_executable(EX01_EMPTY_WINDOW source/examples/ex01_empty_window.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX01_EMPTY_WINDOW glfw)

add_executable(EX02_SHADER_INTRODUCTION source/examples/ex02_shader_introduction.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX02_SHADER_INTRODUCTION glfw)

add_executable(EX03_UNIFORMS source/examples/ex03_uniforms.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX03_UNIFORMS glfw)

add_executable(EX04_VARYINGS source/examples/ex04_varyings.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX04_VARYINGS glfw)

add_executable(EX05_ATTRIBUTES source/examples/ex05_attributes.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX05_ATTRIBUTES glfw)

add_executable(EX06_MULTIPLE_ATTRIBUTES source/examples/ex06_multiple_attributes.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX06_MULTIPLE_ATTRIBUTES glfw)

add_executable(EX07_INTERLEAVED_ATTRIBUTES source/examples/ex07_interleaved_attributes.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX07_INTERLEAVED_ATTRIBUTES glfw)

add_executable(EX08_ELEMENTS source/examples/ex08_elements.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX08_ELEMENTS glfw)

add_executable(EX09_STREAM source/examples/ex09_stream.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX09_STREAM glfw)

add_executable(EX10_MODEL_LOADING source/examples/ex10_model_loading.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX10_MODEL_LOADING glfw)