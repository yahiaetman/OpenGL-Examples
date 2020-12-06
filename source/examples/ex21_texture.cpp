#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <texture/texture-utils.h>

#include <unordered_map>

// This application demonstrates how to create, bind and draw textures on a simple fullscreen triangle.
class TextureApplication : public our::Application {

    our::ShaderProgram program;
    GLuint vertex_array = 0;

    // Since we will have multiple textures in our scene, we will store them in a dictionary
    // and select one to display by its key.
    // Note that textures are OpenGL objects so we identify them via a GLuint.
    std::unordered_map<std::string, GLuint> textures;
    std::string current_texture_name;
    // This integer defines the mip level that we will display.
    int level_of_detail = 0;
    // This will be used to zoom into the texture since they can become very small at high mip levels.
    float zoom = 1;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Textures", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        // The vertex shader will display a fullscreen triangle.
        program.attach("assets/shaders/ex21_texture/fullscreen_triangle.vert", GL_VERTEX_SHADER);
        // The fragment shader will fetch and display a pixel from the given texture based on the fragment coordinate.
        program.attach("assets/shaders/ex21_texture/texel_fetch.frag", GL_FRAGMENT_SHADER);
        program.link();

        // We need a vertex array to call draw so we will create an empty one.
        // Remember that we are using a vertex shader that generates its our vertex data so no need to create any buffers.
        glGenVertexArrays(1, &vertex_array);

        GLuint texture; // Textures are OpenGL objects so we identify them via a GLuint.

        {
            // Here we generate single texture
            glGenTextures(1, &texture);
            // As we already know, OpenGL doesn't care about the C++ datatype of the data we give it as long as we tell it how to correctly interpret it.
            // Here, we will store our data as bytes (uint8) where each 4 bytes represent a color (RGBA).
            // The data are ordered row by row starting from the bottom left pixel.
            uint8_t pixel_data[] = {
                    255,   0,   0, 255,
                      0, 255,   0, 255,
                      0,   0, 255, 255,
                    255, 255,   0, 255,
            };
            // To work with our texture as a 2D texture, we bind it to GL_TEXTURE_2D.
            glBindTexture(GL_TEXTURE_2D, texture);
            // In this function, we tell OpenGL how our data is aligned. This will be used when OpenGL transfers our data from the RAM to the VRAM.
            // Here we wrote that the row alignment is 4 bytes which means that each row starts at an offset that is multiple of 4 bytes.
            // This is always true for any tightly-packed texture data that has 4 components (RGBA) since each row will always be multiple of 4 bytes.
            // However, we can also use 2 or 1 and it would be correct. Actually, there is no advantage of using 4 here.
            // However, 8 will fail if we only have 1 pixel per row (or any odd number of pixels per row).
            // As a general rule, any texture data that is tightly-packed (has no padding between rows) can use an unpack alignment of 1.
            // We only need other values such as 2, 4 or 8 if we have padding between rows.
            // Possible values for this setting is 1, 2, 4 and 8. The default value is 4.
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            // This function sends texture data from the RAM to the VRAM.
            // Parameters:
            // target (GLenum): The texture to which we should send the data. We pick GL_TEXTURE_2D since we will send it to the texture bound to GL_TEXTURE_2D.
            // level (GLint): The mip level in which this data should be stored. Since this data for the texture at its original size, we send it to mip level 0.
            // internalformat (GLint): The format in which the texture data will be stored in the VRAM. Since we have 4 channels (8 bits each) , we will store it as GL_RGBA8.
            // width, height (GLsizei): the width and height of the texture in pixels. Here we picked 2x2 so the data will be stored as 2 rows where each row contains 2 pixels.
            // border (GLint): This does nothing and it must be 0. In old OpenGL, this was used to indicate whether the texture would have a border or not.
            // format (GLenum): this is the format of the data as it is stored in the array "data" on the RAM. Since we have 4 components, we use GL_RGBA.
            // type (GLenum): this is the data type of each component in the array "data" on the RAM. We stored it as uint8 so we pick GL_UNSIGNED_BYTE.
            // data (const void*): this is the pointer to the data on the RAM. The function will read data from this location and send it to the GPU.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
            // This function will generate the mip map for the texture. It will automatically generate all the mip level till we reach a mip level whose size is 1x1 pixel.
            // The mip levels are generate by averaging each 2x2 pixels into 1 pixel in the higher level.
            // Therefore, if mip level (i) has a size WxH, then mip level (i+1) will have a size floor(W/2) x floor(H/2).
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        // Store the texture name in the dictionary
        textures["colors"] = texture;

        {
            glGenTextures(1, &texture);
            // Here, we are creating a local type alias for our color vector which is a specialization of glm vector with 4 uint8 components.
            using color = glm::vec<4, glm::uint8, glm::defaultp>;
            // Then we define some constant colors to use later.
            const color W = {255, 255, 255, 255}, Y = {255, 255, 0, 255},
                    B = {0, 0, 0, 255};
            // Instead of using the data type uint8 and writing each component in the array separately, we use our color data type so each element will be a full color.
            color pixel_data[] = {
                    W, W, Y, Y, Y, Y, W, W,
                    W, Y, Y, B, B, Y, Y, W,
                    Y, Y, B, Y, Y, B, Y, Y,
                    Y, Y, Y, Y, Y, Y, Y, Y,
                    Y, Y, B, Y, Y, B, Y, Y,
                    Y, Y, B, Y, Y, B, Y, Y,
                    W, Y, Y, Y, Y, Y, Y, W,
                    W, W, Y, Y, Y, Y, W, W,
            };
            glBindTexture(GL_TEXTURE_2D, texture);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            // Note that we still used GL_UNSIGNED_BYTE because each component is still a uint8.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        textures["smiley"] = texture;

        {
            glGenTextures(1, &texture);
            // Here we define another local type alias for the color but with 3 components (RGB) instead of 4 (RGBA).
            using color = glm::vec<3, glm::uint8, glm::defaultp>;
            // The colors here has no alpha.
            const color W = {255, 255, 255}, R = {255, 0, 0};
            color pixel_data[] = {
                    W, W, W, W, W,
                    W, W, R, W, W,
                    W, R, R, R, W,
                    W, W, R, W, W,
                    W, W, W, W, W,
            };
            glBindTexture(GL_TEXTURE_2D, texture);
            // Here we must use an alignment of 1 since each row has odd number of bytes.
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            // Since we have no alpha, the internal format is GL_RGB8 and the format is GL_RGB.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 5, 5, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        textures["cross"] = texture;

        {
            glGenTextures(1, &texture);
            const GLuint width = 256, height = 128, tile_size = 32;
            // In this texture, we will store floats instead of bytes.
            float pixel_data[width * height];
            GLuint index = 0;
            // This loop will create a bubble shaped texture.
            for(GLuint y = 0; y < height; ++y){
                for(GLuint x = 0; x < width; ++x){
                    auto tile_coord = 2.0f * glm::fract(glm::vec2(x,y)/static_cast<float>(tile_size)) - 1.0f;
                    auto length2 = glm::dot(tile_coord, tile_coord);
                    pixel_data[index++] = length2 > 1 ? 0.0f : glm::sqrt(1.0f - length2);
                }
            }
            glBindTexture(GL_TEXTURE_2D, texture);
            // We can use 4 since each component has 4 bytes (1 float = 4 bytes of memory).
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            // We only need one component that store a 32-bit float so we use GL_R32F.
            // Also the format will be GL_RED since we have 1 component only per pixel, and its type will be GL_FLOAT.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, pixel_data);
            // Note that the texture will look red since the green and blue components will be 0.
            // If you want the texture to look grayscale, you can use swizzling to rewire the green & blue channels to read their value from the red channel.
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        textures["bubbles"] = texture;

        // The remaining textures are read from files using our function that internally uses the "stb_image" library to read the color data from the files.
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/color-grid.png");
        textures["color-grid"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/monarch.png");
        textures["monarch"] = texture;

        current_texture_name = "color-grid";

        glClearColor(0, 0, 0, 1);
    }

    void onDraw(double deltaTime) override {
        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT);

        GLuint texture = textures[current_texture_name];

        // Unlike other uniforms, we send textures in a special manner.
        // First, we sst the active texture unit. Here, we pick unit 0 which is actually the active unit by default but we still wrote this line for demonstration.
        glActiveTexture(GL_TEXTURE0);
        // When we bind the texture, we also bind it to the active unit. So this texture is now bound to unit 0.
        glBindTexture(GL_TEXTURE_2D, texture);
        // Now, we don't send the texture directly to the uniform variable. Instead, we send the index of the texture unit, and the shader will use the texture bound to that unit.
        program.set("sampler", 0);

        program.set("lod", level_of_detail);
        program.set("zoom", zoom);

        // Now we just draw 3 vertices and the rest of the work is done by the shaders.
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

    void onDestroy() override {
        program.destroy();
        glDeleteVertexArrays(1, &vertex_array);
        for(const auto& [name, texture] : textures)
            glDeleteTextures(1, &texture);
        textures.clear();
    }

    void onImmediateGui(ImGuiIO &io) override {
        ImGui::Begin("Controls");

        if(ImGui::BeginCombo("Texture", current_texture_name.c_str())){
            for(const auto& [name, texture]: textures){
                bool selected = current_texture_name == name;
                if(ImGui::Selectable(name.c_str(), selected))
                    current_texture_name = name;
                if(selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        GLuint texture = textures[current_texture_name];

        GLint width, height;

        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        GLint mipmap_levels = glm::floor(glm::log2(static_cast<float>(glm::max(width, height)))) + 1;

        ImGui::Text("Original Size: %i x %i (mip levels: %i)", width, height, mipmap_levels);

        if(level_of_detail >= mipmap_levels) level_of_detail = mipmap_levels - 1;

        ImGui::DragInt("Level of Detail", &level_of_detail, 1.0f, 0, mipmap_levels-1);

        glGetTexLevelParameteriv(GL_TEXTURE_2D, level_of_detail, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level_of_detail, GL_TEXTURE_HEIGHT, &height);
        ImGui::Text("Current LOD Size: %i x %i", width, height);

        ImGui::DragFloat("Zoom", &zoom, 0.1f, 0, 1000.0f);

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return TextureApplication().run();
}
