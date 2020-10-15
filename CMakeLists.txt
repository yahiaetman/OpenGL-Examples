cmake_minimum_required(VERSION 3.0.0)
project(GFX-LAB VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
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
        source/common/application.cpp source/common/shader.cpp source/common/mesh/mesh-utils.cpp source/common/texture/texture-utils.cpp)

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