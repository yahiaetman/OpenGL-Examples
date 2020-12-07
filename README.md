# OpenGL Examples (CMP205)

This repository contains examples of how to use OpenGL with C++17. It is made for the "Computer Graphics and Man-Machine Interfacing" course (CMPN205) at Cairo University Faculty of Engineering.

**IMPORTANT:**

> While running, make sure that the working directory is the project directory and not the build folder.
> This is necessary since the code expects to run in the same directory where the "assets" folder exist.

## Examples

| Name | Source Code | Documentation |
| ---- | ----------- | ------ |
| Empty Window | [ex01_empty_window.cpp](source/examples/ex01_empty_window.cpp) | :white_check_mark: |
| Shader Introduction | [ex02_shader_introduction.cpp](source/examples/ex02_shader_introduction.cpp) | :white_check_mark: |
| Uniforms | [ex03_uniforms.cpp](source/examples/ex03_uniforms.cpp) | :white_check_mark: |
| Varyings | [ex04_varyings.cpp](source/examples/ex04_varyings.cpp) | :white_check_mark: |
| Attributes | [ex05_attributes.cpp](source/examples/ex05_attributes.cpp) | :white_check_mark: |
| Multiple Attributes | [ex06_multiple_attributes.cpp](source/examples/ex06_multiple_attributes.cpp) | :white_check_mark: |
| Interleaved Attributes | [ex07_interleaved_attributes.cpp](source/examples/ex07_interleaved_attributes.cpp) |  |
| Elements | [ex08_elements.cpp](source/examples/ex08_elements.cpp) |  |
| Stream | [ex09_stream.cpp](source/examples/ex09_stream.cpp) |  |
| Model Loading | [ex10_model_loading.cpp](source/examples/ex10_model_loading.cpp) |  |
| Transformation | [ex11_transformation.cpp](source/examples/ex11_transformation.cpp) | :white_check_mark: |
| Composition | [ex12_composition.cpp](source/examples/ex12_composition.cpp) | :white_check_mark: |
| Camera | [ex13_camera.cpp](source/examples/ex13_camera.cpp) | :white_check_mark: |
| Projection | [ex14_projection.cpp](source/examples/ex14_projection.cpp) | :white_check_mark: |
| Depth Testing | [ex15_depth_testing.cpp](source/examples/ex15_depth_testing.cpp) | :white_check_mark: |
| Face Culling | [ex16_face_culling.cpp](source/examples/ex16_face_culling.cpp) | :white_check_mark: |
| Viewports & Scissors | [ex17_viewports_and_scissors.cpp](source/examples/ex17_viewports_and_scissors.cpp) |  |
| Camera Stacking | [ex18_camera_stacking.cpp](source/examples/ex18_camera_stacking.cpp) |  |
| Ray Casting | [ex19_ray_casting.cpp](source/examples/ex19_ray_casting.cpp) |  |
| Scene Graphs | [ex20_scene_graphs.cpp](source/examples/ex20_scene_graphs.cpp) | :white_check_mark: |
| Texture | [ex21_texture.cpp](source/examples/ex21_texture.cpp) | :white_check_mark: |
| Texture Sampling | [ex22_texture_sampling.cpp](source/examples/ex22_texture_sampling.cpp) | :white_check_mark: |
| Sampler Objects | [ex23_sampler_objects.cpp](source/examples/ex23_sampler_objects.cpp) | :white_check_mark: |
| Displacement | [ex24_displacement.cpp](source/examples/ex24_displacement.cpp) | :white_check_mark: |
| Blending | [ex25_blending.cpp](source/examples/ex25_blending.cpp) | :white_check_mark: |
| Frame Buffer | [ex26_frame_buffer.cpp](source/examples/ex26_frame_buffer.cpp) | :white_check_mark: |
| Postprocessing | [ex27_postprocessing.cpp](source/examples/ex27_postprocessing.cpp) | :white_check_mark: |
| Multiple Render Targets | [ex28_multiple_render_targets.cpp](source/examples/ex28_multiple_render_targets.cpp) |  |
| Light | [ex29_light.cpp](source/examples/ex29_light.cpp) | :white_check_mark: |
| Light Array | [ex30_light_array.cpp](source/examples/ex30_light_array.cpp) | :white_check_mark: |
| Multi-Pass Lighting | [ex31_light_multipass.cpp](source/examples/ex31_light_multipass.cpp) | :white_check_mark: |
| Textured Material | [ex32_textured_material.cpp](source/examples/ex32_textured_material.cpp) | :white_check_mark: |

## Extra Resources

* Tutorials
    * [Learn OpenGL](https://learnopengl.com/)
    * [Open.GL](https://open.gl/introduction)
    * [OpenGL-Tutorial](http://www.opengl-tutorial.org/)
    * [The Book of Shaders](https://thebookofshaders.com/)
* References
    * [OpenGL Wiki](https://www.khronos.org/opengl/wiki/)
    * [OpenGL Reference](https://www.khronos.org/registry/OpenGL-Refpages/gl4/)

## Included Libraries

- [glfw 3.3](https://github.com/glfw/glfw)
- [glad 2](https://github.com/Dav1dde/glad/tree/glad2)
- [glm 0.9.9.8](https://github.com/g-truc/glm)
- [imgui v1.78](https://github.com/ocornut/imgui)
- [json 3.9.1](https://github.com/nlohmann/json)
- [stb](https://github.com/nothings/stb)
- [tinyobjloader v1.0.6](https://github.com/tinyobjloader/tinyobjloader)
- [tinygltf v2.4.0](https://github.com/syoyo/tinygltf) [Not Used Yet]

## License
 [MIT License](LICENSE.md)