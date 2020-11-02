#include "application.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

// Include the Dear ImGui implementation headers
#define IMGUI_IMPL_OPENGL_LOADER_GLAD2
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>

#if !defined(NDEBUG)
// If NDEBUG (no debug) is not defined, enable OpenGL debug messages
#define ENABLE_OPENGL_DEBUG_MESSAGES
#endif

#include "texture/screenshot.h"

// This function will be used to log errors thrown by GLFW
void glfw_error_callback(int error, const char* description){
    std::cerr << "GLFW Error: " << error << ": " << description << std::endl;
}

// This function will be used to log OpenGL debug messages
void GLAPIENTRY opengl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::string _source;
    std::string _type;
    std::string _severity;

    // What is the source of the message
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER: default:
            _source = "UNKNOWN"; break;
    }

    // What is the type of the message (error, warning, etc).
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER"; break;
        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER"; break;
        default:
            _type = "UNKNOWN"; break;
    }

    // How severe is the message
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION"; break;
        default:
            _severity = "UNKNOWN"; break;
    }

    std::cout << "OpenGL Debug Message " << id << " (type: " << _type << ") of " << _severity
    << " raised from " << _source << ": " << message << std::endl;
}

void our::Application::configureOpenGL() {
    // Request that OpenGL is 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Only enable core functionalities (disable features from older OpenGL versions that were removed in 3.3)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Enable forward compatibility with newer OpenGL versions by removing deprecated functionalities
    // This is necessary for some platforms
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //Make window size fixed (User can't resize it)
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //Set Number of sample used in MSAA (0 = Disabled)
    glfwWindowHint(GLFW_SAMPLES, 0);

    //Enable Double Buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    //Set the bit-depths of the frame buffer
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    //Set Bits for Depth Buffer
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    //Set Bits for Stencil Buffer
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    //Set the refresh rate of the window (GLFW_DONT_CARE = Run as fast as possible)
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
}

our::WindowConfiguration our::Application::getWindowConfiguration() {
    return {"OpenGL Application", {1280, 720}, false };
}

// This is the main class function that run the whole application (Initialize, Game loop, House cleaning).
int our::Application::run() {

    // Set the function to call when an error occurs.
    glfwSetErrorCallback(glfw_error_callback);

    // Initialize GLFW and exit if it failed
    if(!glfwInit()){
        std::cerr << "Failed to Initialize GLFW" << std::endl;
        return -1;
    }

    configureOpenGL();                                      // This function sets OpenGL window hints.

    auto win_config = getWindowConfiguration();             // Returns the WindowConfiguration current struct instance.


    // Create a window with the given "WindowConfiguration" attributes.
    // If it should be fullscreen, monitor should point to one of the monitors (e.g. primary monitor), otherwise it should be null
    GLFWmonitor* monitor = win_config.isFullscreen ? glfwGetPrimaryMonitor() : nullptr;
    // The last parameter "share" can be used to share the resources (OpenGL objects) between multiple windows.
    window = glfwCreateWindow(win_config.size.x, win_config.size.y, win_config.title, monitor, nullptr);
    if(!window) {
        std::cerr << "Failed to Create Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);         // Tell GLFW to make the context of our window the main context on the current thread.

    gladLoadGL(glfwGetProcAddress);         // Load the OpenGL functions from the driver

    // Print information about the OpenGL context
    std::cout << "VENDOR          : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "RENDERER        : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "VERSION         : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL VERSION    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

#if defined(ENABLE_OPENGL_DEBUG_MESSAGES)
    // if we have OpenGL debug messages enabled, set the message callback
    glDebugMessageCallback(opengl_callback, nullptr);
    // Then enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    // Then make the output synchronized to the OpenGL commands.
    // This will make sure that OpenGL and the main thread are synchronized such that message callback is called as soon
    // as the command causing it is called. This is useful for debugging but slows down the code execution.
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

    setupCallbacks();
    keyboard.enable(window);
    mouse.enable(window);

    // Start the ImGui context and set dark style (just my preference :D)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Call onInitialize if the application needs to do some custom initialization (such as file loading, object creation, etc).
    onInitialize();

    // The time at which the last frame started. But there was no frames yet, so we'll just pick the current time.
    double last_frame_time = glfwGetTime();

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents(); // Read all the user events and call relevant callbacks.

        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        onImmediateGui(io); // Call to run any required Immediate GUI.

        // If ImGui is using the mouse or keyboard, then we don't want the captured events to affect our keyboard and mouse objects.
        // For example, if you're focusing on an input and writing "W", the keyboard object shouldn't record this event.
        keyboard.setEnabled(!io.WantCaptureKeyboard, window);
        mouse.setEnabled(!io.WantCaptureMouse, window);

        // Render the ImGui commands we called (this doesn't actually draw to the screen yet.
        ImGui::Render();

        // Just in case ImGui changed the OpenGL viewport (the portion of the window to which we render the geometry),
        // we set it back to cover the whole window
        auto frame_buffer_size = getFrameBufferSize();
        glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);

        // Get the current time (the time at which we are starting the current frame).
        double current_frame_time = glfwGetTime();

        // Call onDraw, in which we will draw the current frame, and send to it the time difference between the last and current frame
        onDraw(current_frame_time - last_frame_time);
        last_frame_time = current_frame_time; // Then update the last frame start time (this frame is now the last frame)

#if defined(ENABLE_OPENGL_DEBUG_MESSAGES)
        // Since ImGui causes many messages to be thrown, we are temporarily disabling the debug messages till we render the ImGui
        glDisable(GL_DEBUG_OUTPUT);
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render the ImGui to the framebuffer
#if defined(ENABLE_OPENGL_DEBUG_MESSAGES)
        // Re-enable the debug messages
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

        // If F12 is pressed, take a screenshot
        if(keyboard.justPressed(GLFW_KEY_F12)){
            glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);
            std::stringstream stream;
            auto time = std::time(nullptr);
            auto localtime = std::localtime(&time);
            stream << "screenshots/screenshot-" << std::put_time(localtime, "%Y-%m-%d-%H-%M-%S") << ".png";
            if(our::screenshot_png(stream.str())){
                std::cout << "Screenshot saved to: " << stream.str() << std::endl;
            } else {
                std::cerr << "Failed to save a Screenshot" << std::endl;
            }
        }

        // Swap the frame buffers
        glfwSwapBuffers(window);

        // Update the keyboard and mouse data
        keyboard.update();
        mouse.update();
    }

    // Call for cleaning up
    onDestroy();

    // Shutdown ImGui & destroy the context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Destroy the window
    glfwDestroyWindow(window);

    // And finally terminate GLFW
    glfwTerminate();
    return 0; // Good bye
}

// Sets-up the window callback functions from GLFW to our (Mouse/Keyboard) classes.
void our::Application::setupCallbacks() {

    // We use GLFW to store a pointer to "this" window instance.
    glfwSetWindowUserPointer(window, this);
    // The pointer is then retrieved in the callback function.

    // The second parameter to "glfwSet---Callback" is a function pointer.
    // It is replaced by an inline function -lambda expression- as it is not needed to create
    // a seperate function for it.
    // In the inline function we retrieve the window instance and use it to set our (Mouse/Keyboard) classes values.

    // Keyboard callbacks
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getKeyboard().keyEvent(key, scancode, action, mods);
            app->onKeyEvent(key, scancode, action, mods);
        }
    });

    // mouse position callbacks
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x_position, double y_position){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().CursorMoveEvent(x_position, y_position);
            app->onCursorMoveEvent(x_position, y_position);
        }
    });

    // mouse position callbacks
    glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->onCursorEnterEvent(entered);
        }
    });

    // mouse button position callbacks
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().MouseButtonEvent(button, action, mods);
            app->onMouseButtonEvent(button, action, mods);
        }
    });

    // mouse scroll callbacks
    glfwSetScrollCallback(window, [](GLFWwindow* window, double x_offset, double y_offset){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().ScrollEvent(x_offset, y_offset);
            app->onScrollEvent(x_offset, y_offset);
        }
    });
}
