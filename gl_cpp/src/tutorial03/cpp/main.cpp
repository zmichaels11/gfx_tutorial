/**
 * Tutorial04 - Hello Triangle
 * 
 * Draws a Triangle in the center of the screen.
 * Uses OpenGL 2.0
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
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

    auto window = glfwCreateWindow(640, 480, "Tutorial01", nullptr, nullptr);

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

    auto points = std::array<glm::vec3, 3> ({
        glm::vec3(-1.0F, -1.0F, 0.0F),
        glm::vec3(1.0F, -1.0F, 0.0F),
        glm::vec3(0.0F, 1.0F, 0.0F)});

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points.data(), GL_STATIC_DRAW);

    glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    glPointSize(4.0F);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);        
        glfwPollEvents();
    }
    
    glDeleteBuffers(1, &vbo);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
