#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gfx {
    class Camera {
        glm::vec3 _pos, _target, _up;
        
        void init() noexcept;

    public:
        Camera() noexcept;

        Camera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up) noexcept;

        glm::mat4 getViewMatrix() const noexcept;
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

    inline void Camera::init() noexcept {
    }

    inline glm::mat4 Camera::getViewMatrix() const noexcept {
        return glm::lookAt(_pos, _target, _up);
    }
}
