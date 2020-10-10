#include <application.hpp>

class EmptyWindowApplication : public our::Application {

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Empty Window", {1280, 720}, false };
    }

    void onInitialize() override {
        glClearColor(0.5f, 0.2f, 0.7f, 1.0f);
    }

    void onImmediateGui(ImGuiIO &io) override {
        ImGui::ShowMetricsWindow();
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
    }

};

int main(int argc, char** argv) {
    return EmptyWindowApplication().run();
}
