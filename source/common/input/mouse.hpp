#ifndef MOUSE_H
#define MOUSE_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cstring>

namespace our {

    // A convenience class to read mouse input
    class Mouse {
    private:
        bool enabled; // Is this class enabled (allowed to read user input)
        glm::vec2 currentMousePosition, previousMousePosition;
        bool currentMouseButtons[GLFW_MOUSE_BUTTON_LAST + 1], previousMouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
        glm::vec2 scrollOffset; // Stores mouse wheel scroll amount for this frame

    public:
        // Enable this object and capture current mouse state from window
        void enable(GLFWwindow *window) {
            enabled = true;
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            previousMousePosition = currentMousePosition = glm::vec2((float) x, (float) y);
            for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
                currentMouseButtons[button] = previousMouseButtons[button] = glfwGetMouseButton(window, button);
            }
            scrollOffset = glm::vec2(); // (0, 0)
        }

        // Disable this object and clear the state
        void disable(){
            enabled = false;
            for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
                currentMouseButtons[button] = previousMouseButtons[button] = false;
            }
            scrollOffset = glm::vec2();
        }

        // update the mouse state (mainly moves current frame state to become the previous frame state)
        void update() {
            if(!enabled) return;
            previousMousePosition = currentMousePosition;
            std::memcpy(previousMouseButtons, currentMouseButtons, sizeof(previousMouseButtons));
            scrollOffset = glm::vec2();
        }

        // Current Mouse Position
        [[nodiscard]] const glm::vec2& getMousePosition() const { return currentMousePosition; }

        // How much the mouse moved since the last frame
        [[nodiscard]] glm::vec2 getMouseDelta() const { return currentMousePosition - previousMousePosition; }

        // Is the mouse button currently pressed
        [[nodiscard]] bool isPressed(int button) const { return currentMouseButtons[button]; }

        // Was the mouse button unpressed in the previous frame but became pressed in the current frame
        [[nodiscard]] bool justPressed(int button) const { return currentMouseButtons[button] && !previousMouseButtons[button]; }

        // Was the mouse button pressed in the previous frame but became unpressed in the current frame
        [[nodiscard]] bool justReleased(int button) const { return !currentMouseButtons[button] && previousMouseButtons[button]; }

        // How much the mouse wheel moved in this frame
        [[nodiscard]] const glm::vec2& getScrollOffset() const { return scrollOffset; }

        // Event functions called from GLFW callbacks in "application.cpp"
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

        // Locks the mouse position and hides it (Usually used for FPS games)
        static void lockMouse(GLFWwindow *window) { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
        // If the mouse was locked, unlock it (make it visible and allow it to move)
        static void unlockMouse(GLFWwindow *window) { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }


        [[nodiscard]] bool isEnabled() const { return enabled; }
        void setEnabled(bool enabled, GLFWwindow* window) {
            if(this->enabled != enabled) {
                if (enabled) enable(window);
                else disable();
            }
        }

    };

}

#endif //MOUSE_H
