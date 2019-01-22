/**
 * Tutorial05 - Rotations (OpenGL 4.5)
 * 
 * Draws a Triangle in the center of the screen using shaders.
 * The triangle is then transformed by a rotation matrix.
 * Uses OpenGL 4.5
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
    void errorCallback(int error, const char * desc) {
        std::cerr << "GLFW Error(" << std::dec << error << "): " << desc << std::endl;
    }

    const std::string VERTEX_SHADER = 
        "#version 450\n"        
        "layout (location = 0) in vec3 position;\n"
        "uniform mat4 uModel;\n"

        "void main() {\n"
        "  gl_Position = uModel * vec4(position, 1.0);\n"
        "}";

    const std::string FRAGMENT_SHADER =
        "#version 450\n"
        "layout (location = 0) out vec4 color;\n"

        "void main() {\n"
        "  color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}";

    constexpr GLsizei MAX_INFO_LOG_LENGTH = 1024;

    auto loadShader(GLenum type, const std::string& src) -> decltype(glCreateShader(type)) {
        auto pSrc = src.c_str();
        auto len = static_cast<GLint> (src.length());
        auto shader = glCreateShader(type);

        glShaderSource(shader, 1, &pSrc, &len);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            auto infoLog = std::make_unique<GLchar[]> (MAX_INFO_LOG_LENGTH);

            glGetShaderInfoLog(shader, MAX_INFO_LOG_LENGTH, nullptr, infoLog.get());

            auto msg = std::stringstream();
            msg << "Error compiling shader: " << infoLog.get();
            msg << "\nSource: " << src;

            throw std::runtime_error(msg.str());
        }

        return shader;
    }

    auto linkProgram(const std::vector<GLuint>& shaders) -> decltype(glCreateProgram()) {
        auto program = glCreateProgram();

        for (const auto& shader : shaders) {
            glAttachShader(program, shader);
        }

        glLinkProgram(program);

        for (const auto& shader : shaders) {
            glDetachShader(program, shader);
        }

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            auto infoLog = std::make_unique<GLchar[]> (MAX_INFO_LOG_LENGTH);

            glGetProgramInfoLog(program, MAX_INFO_LOG_LENGTH, nullptr, infoLog.get());

            auto msg = std::stringstream();
            msg << "Error linking program: " << infoLog.get();
            
            throw std::runtime_error(msg.str());
        }

        return program;
    }    

    void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLenum id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        if (GL_DEBUG_TYPE_ERROR == type) {
            std::cerr << "[ERROR]: ";
        } else {
            std::cerr << "[DEBUG]: ";
        }

        std::cerr << message << std::endl;
    }
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(errorCallback);

    if (GLFW_TRUE != glfwInit()) {
        throw std::runtime_error("Failed to init GLFW!");
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    auto window = glfwCreateWindow(640, 480, "Tutorial07", nullptr, nullptr);

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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallback, nullptr);

    GLuint program;
    {
        auto shaders = std::vector<GLuint>();

        shaders.push_back(loadShader(GL_VERTEX_SHADER, VERTEX_SHADER));
        shaders.push_back(loadShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER));

        program = linkProgram(shaders);
    }

    auto points = std::array<glm::vec3, 3> ({
        glm::vec3(-1.0F, -1.0F, 0.0F),
        glm::vec3(1.0F, -1.0F, 0.0F),
        glm::vec3(0.0F, 1.0F, 0.0F)});

    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, sizeof(points), points.data(), GL_STATIC_DRAW);

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    auto uModel = glGetUniformLocation(program, "uModel");

    float t = 0.0F;    

    glClearColor(0.0F, 0.0F, 0.0F, 0.0F);

    while (!glfwWindowShouldClose(window)) {
        auto trModel = glm::rotate(glm::mat4(1.0), t, glm::vec3(0.0F, 0.0F, 1.0F));

        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(trModel));        

        glBindVertexArray(vao);
        glBindVertexBuffer(0, vbo, 0, sizeof(glm::vec3));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);        
        glfwPollEvents();

        t += 0.01F;
    }
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
