#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gfx {
    class Camera {
        glm::vec3 _pos, _target, _up;
        int _windowWidth, _windowHeight;
        float _angleH, _angleV;
        
        void init() noexcept;

    public:
        Camera(int windowWidth, int windowHeight) noexcept;

        Camera(
            int windowWidth, int windowHeight, 
            const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up) noexcept;

        const glm::vec3& getPosition() const noexcept;

        const glm::vec3& getTarget() const noexcept;

        const glm::vec3& getUp() const noexcept;

        glm::mat4 getViewMatrix() const noexcept;
    };

    inline const glm::vec3& Camera::getPosition() const noexcept {
        return _pos;
    }

    inline const glm::vec3& Camera::getTarget() const noexcept {
        return _target;
    }

    inline const glm::vec3& Camera::getUp() const noexcept {
        return _up;
    }

    Camera::Camera(int windowWidth, int windowHeight) noexcept {
        _windowWidth = windowWidth;
        _windowHeight = windowHeight;
        
        _pos = glm::vec3(0.0F, 0.0F, 0.0F);
        _target = glm::vec3(0.0F, 0.0F, -1.0F);
        _up = glm::vec3(0.0F, 1.0F, 0.0F);

        init();
    }

    Camera::Camera(
            int windowWidth, int windowHeight, 
            const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up) noexcept {

        _windowWidth = windowWidth;
        _windowHeight = windowHeight;
        _pos = pos;
        _target = glm::normalize(target);
        _up = glm::normalize(up);

        init();
    }

    inline void Camera::init() noexcept {
        auto hTarget = glm::normalize(glm::vec3(_target.x, 0.0F, _target.z));

        if (hTarget.z >= 0.0F) {
            if (hTarget.x >= 0.0F) {
                _angleH = static_cast<float> (360.0 - glm::degrees(glm::asin(hTarget.z)));
            } else {
                _angleH = static_cast<float> (180.0 + glm::degrees(glm::asin(hTarget.z)));
            }
        } else {
            if (hTarget.x >= 0.0F) {
                _angleH = static_cast<float> (glm::degrees(glm::asin(-hTarget.z)));
            } else {
                _angleH = static_cast<float> (180.0F - glm::degrees(glm::asin(-hTarget.z)));
            }
        }

        _angleV = static_cast<float> (-glm::degrees(glm::asin(_target.y)));        
    }

    inline glm::mat4 Camera::getViewMatrix() const noexcept {
        return glm::lookAt(_pos, _target, _up);
    }
}
