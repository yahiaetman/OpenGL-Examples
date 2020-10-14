#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <texture/texture-utils.h>

#include <unordered_map>

class TextureApplication : public our::Application {

    our::ShaderProgram program;
    GLuint vertex_array;

    std::unordered_map<std::string, GLuint> textures;
    std::string current_texture_name;
    int level_of_detail = 0;
    float zoom = 1;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Textures", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex21_texture/fullscreen_triangle.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex21_texture/texel_fetch.frag", GL_FRAGMENT_SHADER);
        program.link();

        glGenVertexArrays(1, &vertex_array);

        GLuint texture;

        {
            glGenTextures(1, &texture);
            using color = glm::vec<4, glm::uint8, glm::defaultp>;
            const color W = {255, 255, 255, 255}, Y = {255, 255, 0, 255},
                    B = {0, 0, 0, 255};
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        textures["smiley"] = texture;

        {
            glGenTextures(1, &texture);
            using color = glm::vec<3, glm::uint8, glm::defaultp>;
            const color W = {255, 255, 255}, R = {255, 0, 0};
            color pixel_data[] = {
                    W, W, W, W, W,
                    W, W, R, W, W,
                    W, R, R, R, W,
                    W, W, R, W, W,
                    W, W, W, W, W,
            };
            glBindTexture(GL_TEXTURE_2D, texture);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 5, 5, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        textures["cross"] = texture;

        {
            glGenTextures(1, &texture);
            const GLuint width = 256, height = 128, tile_size = 32;
            float pixel_data[width * height];
            GLuint index = 0;
            for(GLuint y = 0; y < height; ++y){
                for(GLuint x = 0; x < width; ++x){
                    auto tile_coord = 2.0f * glm::fract(glm::vec2(x,y)/static_cast<float>(tile_size)) - 1.0f;
                    auto length2 = glm::dot(tile_coord, tile_coord);
                    pixel_data[index++] = length2 > 1 ? 0.0f : glm::sqrt(1.0f - length2);
                }
            }
            glBindTexture(GL_TEXTURE_2D, texture);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, pixel_data);
            //If you want the texture to look grayscale, you can swizzling to rewire the green & blue channels to read their value from the red channel.
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        textures["bubbles"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/color-grid.png");
        textures["color-grid"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;

        current_texture_name = "color-grid";

        glClearColor(0, 0, 0, 1);
    }

    void onDraw(double deltaTime) override {
        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT);

        GLuint texture = textures[current_texture_name];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        program.set("sampler", 0);

        program.set("lod", level_of_detail);
        program.set("zoom", zoom);

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
