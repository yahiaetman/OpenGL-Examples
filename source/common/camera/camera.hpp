#ifndef OUR_CAMERA_HPP
#define OUR_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace our {

    // An enum for the camera projection type
    enum struct CameraType {
        Orthographic,
        Perspective
    };

    // A class that represents a camera
    // Used to generate a view and a projection matrix
    class Camera {
    private:
        // Dirty Flags are programming pattern where we only regenerate some output if:
        // 1- The inputs were changed.
        // 2- The output is needed.
        // Where we have flags for whether the View, Projection & ViewProjection matrices needed to be regenerated.
        static const uint8_t V_DIRTY = 1, P_DIRTY = 2, VP_DIRTY = 4;
        uint8_t dirtyFlags = 0;

        // The camera position, camera forward direction and camera up direction
        glm::vec3 eye = {0, 0, 0}, direction = {0, 0, -1}, up = {0, 1, 0};

        CameraType type = CameraType::Perspective;

        // The field_of_view_y is in radians and is only used for perspective cameras
        // The orthographic_height is only used for orthographic cameras
        float field_of_view_y = glm::radians(90.0f), orthographic_height = 2.0f, aspect_ratio = 1.0f, near = 0.01f, far = 100.0f;

        glm::mat4 V{}, P{}, VP{};

    public:
        Camera(){
            dirtyFlags = V_DIRTY | P_DIRTY | VP_DIRTY;
            up = {0, 1, 0};
        }

        // Setup the camera as a perspective camera
        void setupPerspective(float field_of_view_y, float aspect_ratio, float near, float far){
            this->type = CameraType::Perspective;
            this->field_of_view_y = field_of_view_y;
            this->aspect_ratio = aspect_ratio;
            this->near = near;
            this->far = far;
            dirtyFlags |= P_DIRTY | VP_DIRTY; // Both P & VP need to be regenerated
        }

        // Setup the camera as an orthographic camera
        void setupOrthographic(float orthographic_height, float aspect_ratio, float near, float far){
            this->type = CameraType::Orthographic;
            this->orthographic_height = orthographic_height;
            this->aspect_ratio = aspect_ratio;
            this->near = near;
            this->far = far;
            dirtyFlags |= P_DIRTY | VP_DIRTY; // Both P & VP need to be regenerated
        }

        void setType(CameraType _type){
            if(this->type != _type){
                dirtyFlags |= P_DIRTY | VP_DIRTY;
                this->type = _type;
            }
        }
        void setOrthographicSize(float orthographic_height){
            if(this->orthographic_height != orthographic_height){
                dirtyFlags |= P_DIRTY | VP_DIRTY;
                this->orthographic_height = orthographic_height;
            }
        }
        void setVerticalFieldOfView(float field_of_view_y){
            if(this->field_of_view_y != field_of_view_y){
                dirtyFlags |= P_DIRTY | VP_DIRTY;
                this->field_of_view_y = field_of_view_y;
            }
        }
        void setAspectRatio(float aspect_ratio){
            if(this->aspect_ratio != aspect_ratio){
                dirtyFlags |= P_DIRTY | VP_DIRTY;
                this->aspect_ratio = aspect_ratio;
            }
        }
        void setNearPlane(float near){
            if(this->near != near){
                dirtyFlags |= P_DIRTY | VP_DIRTY;
                this->near = near;
            }
        }
        void setFarPlane(float far){
            if(this->far != far){
                dirtyFlags |= P_DIRTY | VP_DIRTY;
                this->far = far;
            }
        }
        void setEyePosition(glm::vec3 eye){
            if(this->eye != eye){
                dirtyFlags |= V_DIRTY | VP_DIRTY;
                this->eye = eye;
            }
        }
        void setDirection(glm::vec3 direction){
            if(this->direction != direction){
                dirtyFlags |= V_DIRTY | VP_DIRTY;
                this->direction = direction;
            }
        }
        void setTarget(glm::vec3 target){
            glm::vec3 direction = target - eye;
            if(this->direction != direction){
                dirtyFlags |= V_DIRTY | VP_DIRTY;
                this->direction = direction;
            }
        }
        void setUp(glm::vec3 up){
            if(this->up != up){
                dirtyFlags |= V_DIRTY | VP_DIRTY;
                this->up = up;
            }
        }

        glm::mat4 getProjectionMatrix(){
            if(dirtyFlags & P_DIRTY){ // Only regenerate the projection matrix if its flag is dirty
                if(type == CameraType::Orthographic){
                    float half_height = orthographic_height * 0.5f;
                    float half_width = aspect_ratio * half_height;
                    P = glm::ortho(-half_width, half_width, -half_height, half_height, near, far);
                } else {
                    P = glm::perspective(field_of_view_y, aspect_ratio, near, far);
                }
                dirtyFlags &= ~P_DIRTY; // P is no longer dirty
            }
            return P;
        }

        glm::mat4 getViewMatrix(){
            if(dirtyFlags & V_DIRTY){ // Only regenerate the view matrix if its flag is dirty
                V = glm::lookAt(eye, eye + direction, up);
                dirtyFlags &= ~V_DIRTY; // V is no longer dirty
            }
            return V;
        }

        glm::mat4 getVPMatrix(){
            if(dirtyFlags & VP_DIRTY){
                VP = getProjectionMatrix() * getViewMatrix();
                // Note that we called the functions getProjectionMatrix & getViewMatrix instead of directly using V & P
                // to make sure that they are not outdated
                dirtyFlags = 0; // Nothing is dirty anymore
            }
            return VP;
        }

        CameraType getType(){return type;}
        [[nodiscard]] float getVerticalFieldOfView() const {return field_of_view_y;}
        [[nodiscard]] float getHorizontalFieldOfView() const {return field_of_view_y * aspect_ratio;}
        [[nodiscard]] float getOrthographicHeight() const {return orthographic_height;}
        [[nodiscard]] float getOrthographicWidth() const {return orthographic_height * aspect_ratio;}
        [[nodiscard]] float getAspectRatio() const {return aspect_ratio;}
        [[nodiscard]] float getNearPlane() const {return near;}
        [[nodiscard]] float getFarPlane() const {return far;}
        [[nodiscard]] glm::vec3 getEyePosition() const {return eye;}
        [[nodiscard]] glm::vec3 getDirection() const {return direction;}
        [[nodiscard]] glm::vec3 getOriginalUp() const {return up;}

        // Get the directions of the camera coordinates in the world space
        glm::vec3 Right(){
            getViewMatrix();
            return {V[0][0],V[1][0],V[2][0]};
        }
        glm::vec3 Left(){
            getViewMatrix();
            return {-V[0][0],-V[1][0],-V[2][0]};
        }
        glm::vec3 Up(){
            getViewMatrix();
            return {V[0][1],V[1][1],V[2][1]};
        }
        glm::vec3 Down(){
            getViewMatrix();
            return {-V[0][1],-V[1][1],-V[2][1]};
        }
        glm::vec3 Forward(){
            getViewMatrix();
            return {-V[0][2],-V[1][2],-V[2][2]};
        }
        glm::vec3 Backward(){
            getViewMatrix();
            return {V[0][2],V[1][2],V[2][2]};
        }

        // Transform point from world space to normalized device coordinates
        glm::vec3 fromWorldToDeviceSpace(glm::vec3 world){
            glm::vec4 clip = getVPMatrix() * glm::vec4(world, 1.0f);
            return glm::vec3(clip)/clip.w;
            // Note that we must divide by w. This is because of the projection matrix.
        }

        // Transform point from normalized device coordinates to world space
        glm::vec3 fromDeviceToWorldSpace(glm::vec3 device){
            glm::vec4 clip = glm::inverse(getVPMatrix()) * glm::vec4(device, 1.0f);
            return glm::vec3(clip)/clip.w;
            // Note that we must divide by w even though we not going to the NDC space. This is because of the projection matrix.
        }
    };

}

#endif //OUR_CAMERA_HPP
