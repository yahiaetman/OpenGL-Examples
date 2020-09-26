cmake_minimum_required(VERSION 3.0.0)
project(GFX-LAB VERSION 0.1.0)

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

set(PROJECT_SOURCES
        source/main.cpp
        )

include_directories(
        vendor/glfw/include
        vendor/glad/include
        vendor/glm
        vendor/imgui
        vendor/utils
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(${PROJECT_NAME} glfw)