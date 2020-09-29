#include "application.h"

#include <iostream>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>

void error_callback(int error, const char* description){
    std::cerr << "Error " << error << ": " << description << std::endl;
}

our::WindowConfiguration our::Application::configureWindowAndOpenGL() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    return {"OpenGL", {1280, 720}, false };
}

void our::Application::onInitialize() {
    glClearColor(0.5f, 0.25f, 0.75f, 1.0f);
}

void our::Application::onImmediateGui(ImGuiIO&) {
    ImGui::ShowDemoWindow();
}

void our::Application::onDraw(float deltaTime) {
    glClear(GL_COLOR_BUFFER_BIT);
}

void our::Application::run() {

    glfwSetErrorCallback(error_callback);

    if(!glfwInit()){
        std::cerr << "Failed to Initialize GLFW" << std::endl;
        exit(-1);
    }

    auto win_config = configureWindowAndOpenGL();

    GLFWmonitor* monitor = win_config.isFullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window = glfwCreateWindow(win_config.size.x, win_config.size.y, win_config.title, monitor, nullptr);
    if(!window) {
        std::cerr << "Failed to Create Window" << std::endl;
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    gladLoadGL(glfwGetProcAddress);

    setupCallbacks();
    keyboard.enable(window);
    mouse.enable(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    onInitialize();

    double last_frame_time = glfwGetTime();

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        onImmediateGui(io);

        keyboard.setEnabled(!io.WantCaptureKeyboard, window);
        mouse.setEnabled(!io.WantCaptureMouse, window);

        ImGui::Render();

        double current_frame_time = glfwGetTime();

        onDraw(current_frame_time - last_frame_time);

        last_frame_time = current_frame_time;

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        keyboard.update();
        mouse.update();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
}

void our::Application::setupCallbacks() {
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getKeyboard().keyEvent(key, scancode, action, mods);
            app->onKeyEvent(key, scancode, action, mods);
        }
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x_position, double y_position){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().CursorMoveEvent(x_position, y_position);
            app->onCursorMoveEvent(x_position, y_position);
        }
    });

    glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->onCursorEnterEvent(entered);
        }
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().MouseButtonEvent(button, action, mods);
            app->onMouseButtonEvent(button, action, mods);
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double x_offset, double y_offset){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().ScrollEvent(x_offset, y_offset);
            app->onScrollEvent(x_offset, y_offset);
        }
    });
}
