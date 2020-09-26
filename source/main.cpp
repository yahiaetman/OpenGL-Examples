#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>

void error_callback(int error, const char* description){
    std::cerr << "Error " << error << ": " << description << std::endl;
}

int main(int, char**) {

    if(!glfwInit()){
        std::cerr << "Failed to Initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL", nullptr, nullptr);
    if(!window){
        std::cerr << "Failed to Create Window" << std::endl;
        exit(-1);
    }

    glfwMakeContextCurrent(window);

    gladLoadGL(glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        glClearColor(1.0f, 0.5f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
}
