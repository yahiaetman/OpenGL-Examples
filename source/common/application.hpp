#ifndef APPLICATION_H
#define APPLICATION_H

#include <glm/vec2.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

namespace our {

    // This struct handles window attributes: (title, size, isFullscreen).
    struct WindowConfiguration {
        const char* title;
        glm::i16vec2 size;
        bool isFullscreen;
    };

    // This class act as base class for all the Applications covered in the examples.
    // It offers the functionalities needed by all the examples.
    class Application {
    protected:
        GLFWwindow * window = nullptr;      // Pointer to the window created by GLFW using "glfwCreateWindow()".
        Keyboard keyboard;                  // Instance of "our" keyboard class that handles keyboard functionalities.
        Mouse mouse;                        // Instance of "our" mouse class that handles mouse functionalities.

        // Virtual functions to be overrode and change the default behaviour of the application
        // according to the example needs.
        virtual void configureOpenGL();                             // This function sets OpenGL Window Hints in GLFW.
        virtual WindowConfiguration getWindowConfiguration();       // Returns the WindowConfiguration current struct instance.
        virtual void setupCallbacks();                              // Sets-up the window callback functions from GLFW to our (Mouse/Keyboard) classes.

    public:
        virtual void onInitialize(){}                   // Called once before the game loop.
        virtual void onImmediateGui(ImGuiIO& io){}      // Called every frame to draw the Immediate GUI (if any).
        virtual void onDraw(double deltaTime){}         // Called every frame in the game loop passing the time taken to draw the frame "Delta time".
        virtual void onDestroy(){}                      // Called once after the game loop ends for house cleaning.


        // Override these functions to get mouse and keyboard event.
        virtual void onKeyEvent(int key, int scancode, int action, int mods){}      
        virtual void onCursorMoveEvent(double x, double y){}
        virtual void onCursorEnterEvent(int entered){}
        virtual void onMouseButtonEvent(int button, int action, int mods){}
        virtual void onScrollEvent(double x_offset, double y_offset){}

        int run();      // This is the main class function that run the whole application (Initialize, Game loop, House cleaning).

        // Class Getters.
        GLFWwindow* getWindow(){ return window; }
        [[nodiscard]] const GLFWwindow* getWindow() const { return window; }
        Keyboard& getKeyboard() { return keyboard; }
        [[nodiscard]] const Keyboard& getKeyboard() const { return keyboard; }
        Mouse& getMouse() { return mouse; }
        [[nodiscard]] const Mouse& getMouse() const { return mouse; }

        // Get the size of the frame buffer of the window in pixels.
        glm::ivec2 getFrameBufferSize() {
            glm::ivec2 size;
            glfwGetFramebufferSize(window, &(size.x), &(size.y));
            return size;
        }

        // Get the window size. In most cases, it is equal to the frame buffer size.
        // But on some platforms, the framebuffer size may be different from the window size.
        glm::ivec2 getWindowSize() {
            glm::ivec2 size;
            glfwGetWindowSize(window, &(size.x), &(size.y));
            return size;
        }
    };

}


#endif //APPLICATION_H
