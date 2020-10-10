#include <application.hpp>
#include <shader.hpp>


class AttributesApplication : public our::Application {

    our::ShaderProgram program;
    GLuint vertex_array = 0;
    GLuint position_vbo = 0, color_vbo = 0;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Multiple Attributes", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex06_multiple_attributes/multiple_attributes.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex04_varyings/varying_color.frag", GL_FRAGMENT_SHADER);
        program.link();

        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        glGenBuffers(1, &position_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, position_vbo);

        glm::vec3 positions[] = {
                {-0.5, -0.5, 0.0},
                { 0.5, -0.5, 0.0},
                { 0.0,  0.5, 0.0}
        };

        glBufferData(GL_ARRAY_BUFFER, 3*3*sizeof(float), positions, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &color_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, color_vbo);

        glm::vec<4, glm::uint8, glm::defaultp> colors[] = {
                {255, 0, 0, 255},
                { 0, 255, 0, 255},
                { 0,  0, 255, 255}
        };

        glBufferData(GL_ARRAY_BUFFER, 3*4*sizeof(glm::uint8), colors, GL_STATIC_DRAW);

        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, 0, (void*)0);
        glEnableVertexAttribArray(1);

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
        glDeleteBuffers(1, &position_vbo);
        glDeleteBuffers(1, &color_vbo);
    }

};

int main(int argc, char** argv) {
    return AttributesApplication().run();
}
