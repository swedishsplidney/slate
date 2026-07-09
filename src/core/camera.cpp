#include "core/camera.hpp"
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_scancode.h>

namespace slate {

    Camera::Camera(glm::vec3 position)
        : m_position(position),
        m_targetPosition(position),
        m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
        m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
        m_yaw(-90.0f),
        m_pitch(0.0f),
        m_movementSpeed(2.5f),
        m_mouseSensitivity(0.1f)
    {
        updateCameraVectors();
    }

    glm::mat4 Camera::getViewMatrix() const {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    void Camera::processKeyboard(const bool* keyboardState, float deltaTime) {
        float velocity = m_movementSpeed * deltaTime;

        // sdl3 scancodes
        if (keyboardState[SDL_SCANCODE_W]) {
            m_targetPosition += m_front * velocity;
        }
        if (keyboardState[SDL_SCANCODE_S]) {
            m_targetPosition -= m_front * velocity;
        }
        if (keyboardState[SDL_SCANCODE_A]) {
            m_targetPosition -= m_right * velocity;
        }
        if (keyboardState[SDL_SCANCODE_D]) {
            m_targetPosition += m_right * velocity;
        }

        if (keyboardState[SDL_SCANCODE_SPACE]) {
            m_targetPosition += m_worldUp * velocity;
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT]) {
            m_targetPosition -= m_worldUp * velocity;
        }

        float lerpFactor = 10.0 * deltaTime;
        if (lerpFactor > 1.0f) lerpFactor = 1.0f;
        m_position = glm::mix(m_position, m_targetPosition, lerpFactor);
    }

    void Camera::processMouseMovement(float xOffset, float yOffset) {
        xOffset *= m_mouseSensitivity;
        yOffset *= m_mouseSensitivity;

        m_yaw += xOffset;
        m_pitch += yOffset;

        // constrain the pitch
        if (m_pitch > 89.0f) m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;

        // update vectors
        updateCameraVectors();
    }

    void Camera::updateCameraVectors() {
        // calc new front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        // recalc right and up vectors
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

}