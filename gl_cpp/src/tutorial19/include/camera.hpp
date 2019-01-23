#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gfx {
    class Camera {
        glm::vec3 _pos, _target, _up;
        bool _upPressed, _downPressed, _leftPressed, _rightPressed;
        
        void init() noexcept;

    public:
        Camera() noexcept;

        Camera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up) noexcept;        

        glm::mat4 getViewMatrix() const noexcept;

        void onKeyboard(int key, int action) noexcept;

        void update(float stepSize) noexcept;

        const glm::vec3& getPosition() const noexcept;

        void getPosition(float& x, float& y, float& z) const noexcept;
    };

    Camera::Camera() noexcept {
        _pos = glm::vec3(0.0F, 0.0F, 0.0F);
        _target = glm::vec3(0.0F, 0.0F, -1.0F);
        _up = glm::vec3(0.0F, 1.0F, 0.0F);

        init();
    }

    Camera::Camera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up) noexcept {
        _pos = pos;
        _target = glm::normalize(target);
        _up = glm::normalize(up);

        init();
    }

    inline const glm::vec3& Camera::getPosition() const noexcept {
        return _pos;
    }

    inline void Camera::getPosition(float& x, float& y, float& z) const noexcept {
        x = _pos.x;
        y = _pos.y;
        z = _pos.z;
    }

    inline void Camera::init() noexcept {
        auto hTarget = glm::normalize(glm::vec3(_target.x, 0.0F, _target.z));   

        _leftPressed = false;
        _rightPressed = false;
        _upPressed = false;
        _downPressed = false;
    }

    inline glm::mat4 Camera::getViewMatrix() const noexcept {
        return glm::lookAt(_pos, _target, _up);
    }

    inline void Camera::onKeyboard(int key, int action) noexcept {
        switch (key) {
            case GLFW_KEY_UP:
                _upPressed = (GLFW_PRESS == action);
                break;
            case GLFW_KEY_DOWN:
                _downPressed = (GLFW_PRESS == action);
                break;
            case GLFW_KEY_LEFT:
                _leftPressed = (GLFW_PRESS == action);
                break;
            case GLFW_KEY_RIGHT:
                _rightPressed = (GLFW_PRESS == action);
                break;
            default:
                break;
        }
    }

    inline void Camera::update(float stepSize) noexcept {
        auto step = glm::vec3(0.0F);

        if (_upPressed) {
            step = (_target * stepSize);
        } else if (_downPressed) {
            step = _target * -stepSize;
        }

        if (_leftPressed) {
            step = glm::normalize(glm::cross(_target, _up)) * stepSize;            
        } else if (_rightPressed) {
            step = glm::normalize(glm::cross(_up, _target)) * stepSize;
        }        

        _pos += step;
    }
}
