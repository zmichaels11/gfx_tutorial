#pragma once

#include <GL/glew.h>

namespace gfx {
    namespace util {
        constexpr GLsizei alignUp(GLsizei a, GLsizei b) {
            return (a + b - 1) / b * b;
        }
    }
}