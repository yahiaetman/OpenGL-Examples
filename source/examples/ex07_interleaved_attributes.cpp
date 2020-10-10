#include <application.hpp>
#include <shader.hpp>


class AttributesApplication : public our::Application {

    our::ShaderProgram program;
    GLuint vertex_array = 0, vertex_buffer = 0;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Interleaved Attributes", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex06_multiple_attributes/multiple_attributes.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex04_varyings/varying_color.frag", GL_FRAGMENT_SHADER);
        program.link();

        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

#define USE_VERTEX_STRUCT
#ifndef USE_VERTEX_STRUCT
        float data[] = {
                -0.5, -0.5, 0.0,    1.0, 0.0, 0.0, 1.0,
                 0.5, -0.5, 0.0,    0.0, 1.0, 0.0, 1.0,
                 0.0,  0.5, 0.0,    0.0, 0.0, 1.0, 1.0
        };
        glBufferData(GL_ARRAY_BUFFER, 3*7*sizeof(float), data, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, false, 7*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, 7*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
#else
        struct Vertex {
            glm::vec3 position;
            glm::vec<4, glm::uint8, glm::defaultp> color;
        };
        Vertex data[] = {
                {{-0.5, -0.5, 0.0},{255, 0, 0, 255}},
                {{0.5, -0.5, 0.0},{0, 255, 0, 255}},
                {{0.0,  0.5, 0.0},{0, 0, 255, 255}}
        };
        glBufferData(GL_ARRAY_BUFFER, 3*sizeof(Vertex), data, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);
#endif

        glBindVertexArray(0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

    void onDestroy() override {
        program.destroy();
        glDeleteVertexArrays(1, &vertex_array);
        glDeleteBuffers(1, &vertex_buffer);
    }

};

int main(int argc, char** argv) {
    return AttributesApplication().run();
}
