/**
 * Tutorial04 - Hello Dot
 * 
 * Draws a single point in the middle of the screen
 * Uses OpenGL 2.0
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <glm/glm.hpp>

namespace {
    void errorCallback(int error, const char * desc) {
        std::cerr << "GLFW Error(" << std::dec << error << "): " << desc << std::endl;
    }
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(errorCallback);

    if (GLFW_TRUE != glfwInit()) {
        throw std::runtime_error("Failed to init GLFW!");
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    auto window = glfwCreateWindow(640, 480, "Tutorial02", nullptr, nullptr);

    if (nullptr == window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLenum glErr = glewInit();
    if (glErr) {
        auto msg = std::stringstream();

        msg << "Failed to init GLEW: " << glewGetErrorString(glErr);

        throw std::runtime_error(msg.str());
    }

    auto point = glm::vec3(0.0F, 0.0F, 0.0F);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point), &point, GL_STATIC_DRAW);

    glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    glPointSize(4.0F);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(GL_POINTS, 0, 1);

        glfwSwapBuffers(window);        
        glfwPollEvents();
    }
    
    glDeleteBuffers(1, &vbo);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
