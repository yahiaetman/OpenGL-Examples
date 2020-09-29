#ifndef MOUSE_H
#define MOUSE_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cstring>

namespace our {

    class Mouse {
    private:
        bool enabled;
        glm::vec2 currentMousePosition, previousMousePosition;
        bool currentMouseButtons[GLFW_MOUSE_BUTTON_LAST + 1], previousMouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
        glm::vec2 scrollOffset;

    public:
        void enable(GLFWwindow *window) {
            enabled = true;
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            previousMousePosition = currentMousePosition = glm::vec2((float) x, (float) y);
            for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
                currentMouseButtons[button] = previousMouseButtons[button] = glfwGetMouseButton(window, button);
            }
            scrollOffset = glm::vec2();
        }

        void disable(){
            enabled = false;
            for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
                currentMouseButtons[button] = previousMouseButtons[button] = false;
            }
            scrollOffset = glm::vec2();
        }

        void update() {
            if(!enabled) return;
            previousMousePosition = currentMousePosition;
            std::memcpy(previousMouseButtons, currentMouseButtons, sizeof(previousMouseButtons));
            scrollOffset = glm::vec2();
        }

        const glm::vec2& getMousePosition() const { return currentMousePosition; }

        glm::vec2 getMouseDelta() const { return currentMousePosition - previousMousePosition; }

        bool isPressed(int button) const { return currentMouseButtons[button]; }

        bool justPressed(int button) const { return currentMouseButtons[button] && !previousMouseButtons[button]; }

        bool justReleased(int button) const { return !currentMouseButtons[button] && previousMouseButtons[button]; }

        const glm::vec2& getScrollOffset() const { return scrollOffset; }

        void CursorMoveEvent(double x_position, double y_position) {
            if(!enabled) return;
            currentMousePosition.x = (float) x_position;
            currentMousePosition.y = (float) y_position;
        }

        void MouseButtonEvent(int button, int action, int) {
            if(!enabled) return;
            if (action == GLFW_PRESS) currentMouseButtons[button] = true;
            else if (action == GLFW_RELEASE) currentMouseButtons[button] = false;
        }

        void ScrollEvent(double x_offset, double y_offset) {
            if(!enabled) return;
            scrollOffset.x += x_offset;
            scrollOffset.y += y_offset;
        }

        static void lockMouse(GLFWwindow *window) { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
        static void unlockMouse(GLFWwindow *window) { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

        bool isEnabled() const { return enabled; }
        void setEnabled(bool enabled, GLFWwindow* window) {
            if(this->enabled != enabled)
                if(enabled) enable(window);
                else disable();
        }

    };

}

#endif //MOUSE_H
