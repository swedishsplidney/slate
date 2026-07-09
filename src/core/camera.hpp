#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace slate {

    class Camera {
    public:
        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

        // return view matrix
        glm::mat4 getViewMatrix() const;

        // process kb inputs
        void processKeyboard(const bool* keyboardState, float deltaTime);

        // process movement layers
        void processMouseMovement(float xOffset, float yOffset);

    private:
        // cam vectors
        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        glm::vec3 m_worldUp;

        // euler angles
        float m_yaw;
        float m_pitch;

        // cam options
        float m_movementSpeed;
        float m_mouseSensitivity;

        // recalc vectors
        void updateCameraVectors();

        glm::vec3 m_targetPosition;
    };

}