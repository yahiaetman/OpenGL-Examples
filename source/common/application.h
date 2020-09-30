#ifndef APPLICATION_H
#define APPLICATION_H

#include <glm/vec2.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "input/keyboard.h"
#include "input/mouse.h"

namespace our {

    struct WindowConfiguration {
        const char* title;
        glm::i16vec2 size;
        bool isFullscreen;
    };

    class Application {
    protected:
        GLFWwindow * window = nullptr;
        Keyboard keyboard;
        Mouse mouse;

        virtual void configureOpenGL();
        virtual WindowConfiguration getWindowConfiguration();
        virtual void setupCallbacks();

    public:
        virtual void onInitialize(){}
        virtual void onImmediateGui(ImGuiIO& io){}
        virtual void onDraw(double deltaTime){}
        virtual void onDestroy(){}

        virtual void onKeyEvent(int key, int scancode, int action, int mods){}
        virtual void onCursorMoveEvent(double x, double y){}
        virtual void onCursorEnterEvent(int entered){}
        virtual void onMouseButtonEvent(int button, int action, int mods){}
        virtual void onScrollEvent(double x_offset, double y_offset){}

        int run();

        GLFWwindow* getWindow(){ return window; }
        [[nodiscard]] const GLFWwindow* getWindow() const { return window; }
        Keyboard& getKeyboard() { return keyboard; }
        [[nodiscard]] const Keyboard& getKeyboard() const { return keyboard; }
        Mouse& getMouse() { return mouse; }
        [[nodiscard]] const Mouse& getMouse() const { return mouse; }

        glm::ivec2 getFrameBufferSize() {
            glm::ivec2 size;
            glfwGetFramebufferSize(window, &(size.x), &(size.y));
            return size;
        }
    };

}


#endif //APPLICATION_H
