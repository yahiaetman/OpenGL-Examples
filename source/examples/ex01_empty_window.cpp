#include <application.hpp>

// This exercise Window Application that derives from "Application" parent
class EmptyWindowApplication : public our::Application {

    // This overriden function sets the window configuration params struct (title, size, isFullscreen).
    our::WindowConfiguration getWindowConfiguration() override {
        return { "Empty Window", {1280, 720}, false };
    }

    // onInitialize() function is called once before the application loop
    void onInitialize() override {

        //Specify the red, green, blue, and alpha values used when the color buffers are cleared.
        //Here the clear color is Magenta
        glClearColor(0.5f, 0.2f, 0.7f, 1.0f);
    }

    void onImmediateGui(ImGuiIO &io) override {
        // Shows a metric window just as an example of ImGui operations
        ImGui::ShowMetricsWindow();
    }

    // onDraw(deltaTime) function is called every frame 
    void onDraw(double deltaTime) override {
        //At the start of frame we want to clear the screen. Otherwise we would still see the results from the previous frame.
        glClear(GL_COLOR_BUFFER_BIT);
    }

};

// Example Entry point
int main(int argc, char** argv) {
    
    // Creates an instance of EmptyWindowApplication and call run on this instance
    return EmptyWindowApplication().run();
}
