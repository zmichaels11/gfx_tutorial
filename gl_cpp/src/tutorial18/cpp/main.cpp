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

#include "camera.hpp"
#include "texture.hpp"
#include "util.hpp"

namespace {
    void errorCallback(int error, const char * desc) {
        std::cerr << "GLFW Error(" << std::dec << error << "): " << desc << std::endl;
    }

    const std::string VERTEX_SHADER = 
        "#version 450\n"        
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec2 texcoord;\n"
        "layout (location = 2) in vec3 normal;\n"        
        "layout (location = 0) out vec2 vTexCoord;\n"
        "layout (location = 1) out vec3 vNormal;\n"

        "layout (binding = 0, std140) uniform CameraData {\n"
        "  mat4 mvp;\n"
        "  mat4 world;\n"
        "} uCamera;\n"

        "void main() {\n"
        "  gl_Position = uCamera.mvp * vec4(position, 1.0);\n"        
        "  vTexCoord = texcoord;\n"
        "  vNormal = mat3(uCamera.world) * normal;\n"
        "}";

    const std::string FRAGMENT_SHADER =
        "#version 450\n"
        "layout (location = 0) in vec2 vTexCoord;\n"
        "layout (location = 1) in vec3 vNormal;\n"
        "layout (location = 0) out vec4 fColor;\n"

        "uniform sampler2D uImage;\n"

        "layout (binding = 0, std140) uniform CameraData {\n"
        "  mat4 mvp;\n"
        "  mat4 world;\n"
        "} uCamera;\n"

        "layout (binding = 1, std140) uniform SunData {\n"
        "  vec4 color;\n"
        "  vec4 direction;\n"
        "  float ambientIntensity;\n"
        "  float diffuseIntensity;\n"
        "} uSun;\n"

        "void main() {\n"
        "  vec4 ambientColor = vec4(uSun.color.rgb * uSun.ambientIntensity, 1.0);\n"
        "  float diffuseFactor = dot(normalize(vNormal), -uSun.direction.xyz);\n"
        "  vec4 diffuseColor;\n"

        "  if (diffuseFactor > 0.0) {\n"
        "    diffuseColor = vec4(uSun.color.rgb * uSun.diffuseIntensity * diffuseFactor, 1.0);\n"
        "  } else {\n"
        "    diffuseColor = vec4(0.0);\n"
        "  }\n"
        "  fColor = texture(uImage, vTexCoord) * (ambientColor + diffuseColor);\n"        
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

    auto window = glfwCreateWindow(640, 480, "Tutorial18", nullptr, nullptr);

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

    struct Vertex {
        glm::vec3 position;
        glm::vec2 texcoord;
        glm::vec3 normal;
    };

    auto points = std::vector<Vertex> ();
    points.push_back({ glm::vec3(-1.0F, -1.0F, 0.5773F), glm::vec2(0.0F, 0.0F), glm::vec3(0.0F) });
    points.push_back({ glm::vec3(0.0F, -1.0F, -1.15475F), glm::vec2(0.5F, 0.0F), glm::vec3(0.0F) });
    points.push_back({ glm::vec3(1.0F, -1.0F, 0.5773F), glm::vec2(1.0F, 0.0F), glm::vec3(0.0F) });
    points.push_back({ glm::vec3(0.0F, 1.0F, 0.0F), glm::vec2(0.5F, 1.0F), glm::vec3(0.0F) });

    auto indices = std::array<glm::u16, 12> ({
            0, 3, 1,
            1, 3, 2,
            2, 3, 0,
            0, 1, 2
        });

    for (unsigned int i = 0; i < indices.size(); i += 3) {
        auto idx0 = indices[i];
        auto idx1 = indices[i + 1];
        auto idx2 = indices[i + 2];

        auto& p0 = points[idx0];
        auto& p1 = points[idx1];
        auto& p2 = points[idx2];

        auto v1 = p1.position - p0.position;
        auto v2 = p2.position - p0.position;
        auto normal = glm::normalize(glm::cross(v1, v2));
        
        p0.normal += normal;
        p1.normal += normal;
        p2.normal += normal;
    }    

    for (auto& p : points) {
        p.normal = glm::normalize(p.normal);
    }

    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW);

    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glNamedBufferData(ibo, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    struct UBOCameraT {
        glm::mat4 mvp;
        glm::mat4 world;
    };

    struct UBOSunT {        
        glm::vec4 color;
        glm::vec4 direction;
        glm::float32 ambientIntensity;
        glm::float32 diffuseIntensity;
    };

    GLint uboAlignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlignment);

    auto alignedSizeofUBOCameraT = gfx::util::alignUp(sizeof(UBOCameraT), uboAlignment);
    auto alignedSizeofUBOSunT = gfx::util::alignUp(sizeof(UBOSunT), uboAlignment);
    auto totalSizeofUBO = alignedSizeofUBOCameraT + alignedSizeofUBOSunT;

    GLuint ubo;
    glCreateBuffers(1, &ubo);
    glNamedBufferStorage(ubo, totalSizeofUBO, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    UBOCameraT * pCameraData;
    UBOSunT * pSunData;
    {
        auto pBaseData = reinterpret_cast<glm::u8 * > (glMapNamedBufferRange(ubo, 0, totalSizeofUBO, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

        pCameraData = reinterpret_cast<UBOCameraT *> (pBaseData);
        pSunData = reinterpret_cast<UBOSunT *> (pBaseData + alignedSizeofUBOCameraT);
    }

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(vao, 1, 0);
    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float));
    glVertexArrayAttribBinding(vao, 2, 0);
    
    auto uImage = glGetUniformLocation(program, "uImage");

    float t = 0.0F;    

    glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
    glEnable(GL_DEPTH_TEST);

    struct UserDataT {
        std::unique_ptr<gfx::Camera> pCamera;
        float ambientIntensity;
    } userData;

    userData.pCamera = std::make_unique<gfx::Camera>();
    userData.ambientIntensity = 0.1F;

    glfwSetWindowUserPointer(window, &userData);
    glfwSetKeyCallback(window, [](auto pWindow, auto key, auto scancode, auto action, auto mods) {
        auto pUserData = reinterpret_cast<UserDataT * > (glfwGetWindowUserPointer(pWindow));

        pUserData->pCamera->onKeyboard(key, action);
        
        switch (key) {            
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
                break;
            case GLFW_KEY_A:
                pUserData->ambientIntensity += 0.05F;
                break;
            case GLFW_KEY_S:
                pUserData->ambientIntensity -= 0.05F;
                break;
        }
    });

    auto pTexture = std::make_unique<gfx::Texture> (GL_TEXTURE_2D, "data/test.png");

    while (!glfwWindowShouldClose(window)) {
        auto trTrans = glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, -5.0F));
        auto trRotate = glm::rotate(glm::mat4(1.0F), t, glm::vec3(0.0F, 1.0F, 0.0F));        
        auto trProj = glm::perspective(glm::radians(90.0F), 4.0F / 3.0F, 1.0F, 100.0F);
        auto trModel = trTrans * trRotate;
        auto trView = userData.pCamera->getViewMatrix();
        auto trMv = trView * trModel;
        
        pCameraData->mvp = trProj * trMv;
        pCameraData->world = glm::transpose(glm::inverse(trMv));
        pSunData->color = glm::vec4(1.0F);
        pSunData->direction = glm::vec4(1.0F, 0.0F, 0.0F, 1.0F);
        pSunData->ambientIntensity = userData.ambientIntensity;
        pSunData->diffuseIntensity = 0.75F;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(program);        
        glUniform1i(uImage, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, alignedSizeofUBOCameraT);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo, alignedSizeofUBOCameraT, alignedSizeofUBOSunT);

        pTexture->bind(0);        

        glBindVertexArray(vao);
        glBindVertexBuffer(0, vbo, 0, sizeof(Vertex));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, 0);

        glfwSwapBuffers(window);        
        glfwPollEvents();

        userData.pCamera->update(0.1F);

        t += 0.01F;
    }

    pTexture = nullptr;
    
    glDeleteVertexArrays(1, &vao);    
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &ubo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
