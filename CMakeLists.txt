cmake_minimum_required(VERSION 3.0.0)   # Selects the minimum version of CMake required to run this file
project(GFX-LAB VERSION 0.1.0)          # Here we select the project name adn version

# Here we select C++17 with all the standards required and all compiler-specific extensions disabled
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# These are the options we select for building GLFW as a library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)        # Don't build Documentation
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)       # Don't build Tests
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)    # Don't build Examples
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)           # Don't build Installation Information
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)     # Add variables to use High Performance Graphics Card if available
add_subdirectory(vendor/glfw)                       # Build the GLFW project to use later as a library

# A variable with all the source files of GLAD
set(GLAD_SOURCE vendor/glad/src/gl.c)
# A variables with all the source files of Dear ImGui
set(IMGUI_SOURCES
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/imgui_impl/imgui_impl_glfw.cpp
        vendor/imgui/imgui_impl/imgui_impl_opengl3.cpp
        )

# Combine all vendor source files together into a single variable
set(VENDOR_SOURCES ${GLAD_SOURCE} ${IMGUI_SOURCES})

# A variable with all our source files that are common between executable targets (examples)
set(COMMON_SOURCES
        source/common/application.cpp
        source/common/shader.cpp
        source/common/mesh/mesh-utils.cpp
        source/common/texture/texture-utils.cpp)

# Define the directories in which to search for the included headers
include_directories(
        source/common
        vendor/glfw/include
        vendor/glad/include
        vendor/glm
        vendor/imgui
        vendor/utils
)

# For each example, we add an executable target
# Each target compiles one example source file and the common & vendor source files
# Then we link GLFW with each target
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

add_executable(EX11_TRANSFORMATION source/examples/ex11_transformation.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX11_TRANSFORMATION glfw)

add_executable(EX12_COMPOSITION source/examples/ex12_composition.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX12_COMPOSITION glfw)

add_executable(EX13_CAMERA source/examples/ex13_camera.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX13_CAMERA glfw)

add_executable(EX14_PROJECTION source/examples/ex14_projection.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX14_PROJECTION glfw)

add_executable(EX15_DEPTH_TESTING source/examples/ex15_depth_testing.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX15_DEPTH_TESTING glfw)

add_executable(EX16_FACE_CULLING source/examples/ex16_face_culling.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX16_FACE_CULLING glfw)

add_executable(EX17_VIEWPORTS_SCISSORS source/examples/ex17_viewports_and_scissors.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX17_VIEWPORTS_SCISSORS glfw)

add_executable(EX18_CAMERA_STACKING source/examples/ex18_camera_stacking.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX18_CAMERA_STACKING glfw)

add_executable(EX19_RAY_CASTING source/examples/ex19_ray_casting.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX19_RAY_CASTING glfw)

add_executable(EX20_SCENE_GRAPHS source/examples/ex20_scene_graphs.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX20_SCENE_GRAPHS glfw)

add_executable(EX21_TEXTURE source/examples/ex21_texture.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX21_TEXTURE glfw)

add_executable(EX22_TEXTURE_SAMPLING source/examples/ex22_texture_sampling.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX22_TEXTURE_SAMPLING glfw)

add_executable(EX23_SAMPLER_OBJECTS source/examples/ex23_sampler_objects.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX23_SAMPLER_OBJECTS glfw)

add_executable(EX24_DISPLACEMENT source/examples/ex24_displacement.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX24_DISPLACEMENT glfw)

add_executable(EX25_BLENDING source/examples/ex25_blending.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX25_BLENDING glfw)

add_executable(EX26_FRAME_BUFFER source/examples/ex26_frame_buffer.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX26_FRAME_BUFFER glfw)

add_executable(EX27_POSTPROCESSING source/examples/ex27_postprocessing.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX27_POSTPROCESSING glfw)

add_executable(EX28_MULTIPLE_RENDER_TARGETS source/examples/ex28_multiple_render_targets.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX28_MULTIPLE_RENDER_TARGETS glfw)

add_executable(EX29_LIGHT source/examples/ex29_light.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX29_LIGHT glfw)

add_executable(EX30_LIGHT_ARRAY source/examples/ex30_light_array.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX30_LIGHT_ARRAY glfw)

add_executable(EX31_LIGHT_MULTIPASS source/examples/ex31_light_multipass.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX31_LIGHT_MULTIPASS glfw)

add_executable(EX32_TEXTURED_MATERIAL source/examples/ex32_textured_material.cpp ${COMMON_SOURCES} ${VENDOR_SOURCES})
target_link_libraries(EX32_TEXTURED_MATERIAL glfw)