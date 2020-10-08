#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <GLFW/glfw3.h>
#include <cstring>

namespace our {

    class Keyboard {
    private:
        bool enabled;
        bool currentKeyStates[GLFW_KEY_LAST + 1];
        bool previousKeyStates[GLFW_KEY_LAST + 1];

    public:
        void enable(GLFWwindow* window){
            enabled = true;
            for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++){
                currentKeyStates[key] = previousKeyStates[key] = glfwGetKey(window, key);
            }
        }

        void disable(){
            for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++){
                currentKeyStates[key] = previousKeyStates[key] = false;
            }
        }

        void update(){
            if(!enabled) return;
            std::memcpy(previousKeyStates, currentKeyStates, sizeof(previousKeyStates));
        }
        void keyEvent(int key, int, int action, int){
            if(!enabled) return;
            if(action == GLFW_PRESS){
                currentKeyStates[key] = true;
            } else if(action == GLFW_RELEASE){
                currentKeyStates[key] = false;
            }
        }

        [[nodiscard]] bool isPressed(int key) const {return currentKeyStates[key]; }
        [[nodiscard]] bool justPressed(int key) const {return currentKeyStates[key] && !previousKeyStates[key];}
        [[nodiscard]] bool justReleased(int key) const {return !currentKeyStates[key] && previousKeyStates[key];}

        [[nodiscard]] bool isEnabled() const { return enabled; }
        void setEnabled(bool enabled, GLFWwindow* window) {
            if(this->enabled != enabled) {
                if (enabled) enable(window);
                else disable();
            }
        }
    };

}

#endif //KEYBOARD_H
