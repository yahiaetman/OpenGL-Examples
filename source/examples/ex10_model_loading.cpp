#include <application.h>
#include <shader.h>
#include <imgui-utils/utils.h>

#include <mesh/single-buffer-mesh.h>
#include <mesh/common-vertex-types.h>

struct Vertex {
    glm::vec3 position;
    glm::vec<4, glm::uint8, glm::defaultp> color;
};

class MeshApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh quad, model;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Model Loading", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex06_multiple_attributes/multiple_attributes.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex04_varyings/varying_color.frag", GL_FRAGMENT_SHADER);
        program.link();

        quad.create({
            {
                {0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position), false},
                {1, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color), false},
            }
        });
        quad.setVertexData<Vertex>(0, {
           {{-0.5, -0.5, 0},{255,   0,   0, 255}},
           {{ 0.5, -0.5, 0},{  0, 255,   0, 255}},
           {{ 0.5,  0.5, 0},{  0,   0, 255, 255}},
           {{-0.5,  0.5, 0},{255, 255,   0, 255}}
        },GL_STATIC_DRAW);
        quad.setElementData<GLuint>({
            0, 1, 2,
            2, 3, 0
        },GL_STATIC_DRAW);

        our::Mesh::loadOBJ(model, "assets/models/Suzanne/Suzanne.obj");

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void onDraw(double deltaTime) override {


        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        model.draw();

    }

    void onDestroy() override {
        program.destroy();
        quad.destroy();
        model.destroy();
    }

};

int main(int argc, char** argv) {
    return MeshApplication().run();
}
