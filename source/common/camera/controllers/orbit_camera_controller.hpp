#ifndef OUR_ORBIT_CAMERA_CONTROLLER_HPP
#define OUR_ORBIT_CAMERA_CONTROLLER_HPP

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

#include <camera/camera.hpp>
#include <application.hpp>

namespace our {

    // Allows you to control the camera in an orbit motion around a point in world space
    class OrbitCameraController {
    private:
        Application* app;
        Camera* camera;

        float yaw, pitch, distance;
        glm::vec3 origin;
        float yaw_sensitivity, pitch_sensitivity, distance_sensitivity;

        bool mouse_locked = false;

    public:
        void initialize(Application* application, Camera* camera){
            this->app = application;
            this->camera = camera;
            origin = {0,0,0};
            yaw_sensitivity = pitch_sensitivity = 0.01f;
            distance_sensitivity = 2.0f;
        }

        void release(){
            if(mouse_locked) {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }

        void update(double){
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked){
                app->getMouse().lockMouse(app->getWindow());
                mouse_locked = true;
            } else if(!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked) {
                app->getMouse().unlockMouse(app->getWindow());
                mouse_locked = false;
            }

            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1)){
                glm::vec2 delta = app->getMouse().getMouseDelta();
                pitch += delta.y * pitch_sensitivity;
                yaw += delta.x * yaw_sensitivity;
            }

            if(pitch < -glm::half_pi<float>() * 0.99f) pitch = -glm::half_pi<float>() * 0.99f;
            if(pitch >  glm::half_pi<float>() * 0.99f) pitch  = glm::half_pi<float>() * 0.99f;
            yaw = glm::wrapAngle(yaw);

            distance += (float)app->getMouse().getScrollOffset().y * distance_sensitivity;
            if(distance < 0) distance = 0;

            camera->setEyePosition(origin + distance * (glm::vec3(glm::cos(yaw), 0, -glm::sin(yaw)) * glm::cos(pitch) + glm::vec3(0, glm::sin(pitch), 0)));
            camera->setTarget(origin);
        }

        float getYaw(){return yaw;}
        float getPitch(){return pitch;}
        float getDistance(){return distance;}
        glm::vec3 getOrigin(){return origin;}

        float getYawSensitivity(){return yaw_sensitivity;}
        float getPitchSensitivity(){return pitch_sensitivity;}
        float getDistanceSensitivity(){return distance_sensitivity;}

        void setYaw(float _yaw){
            this->yaw = glm::wrapAngle(_yaw);
        }
        void setPitch(float _pitch){
            const float v = 0.99f*glm::pi<float>()/2;
            if(_pitch > v) _pitch = v;
            else if(_pitch < -v) _pitch = -v;
            this->pitch = _pitch;
        }
        void setDistance(float _distance){
            this->distance = glm::max(0.0f, _distance);
        }
        void setOrigin(glm::vec3 _origin){
            this->origin = _origin;
        }

        void setYawSensitivity(float sensitivity){this->yaw_sensitivity = sensitivity;}
        void setPitchSensitivity(float sensitivity){this->pitch_sensitivity = sensitivity;}
        void setDistanceSensitivity(float sensitivity){this->distance_sensitivity = sensitivity;}

    };

}

#endif //OUR_ORBIT_CAMERA_CONTROLLER_HPP
