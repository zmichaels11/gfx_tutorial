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
        "layout (location = 2) out vec3 vWorldPos;\n"

        "layout (binding = 0, std140) uniform Data {\n"
        "  mat4 mvp;\n"
        "  mat4 normal;\n"
        "  mat4 world;\n"
        "  vec4 color;\n"
        "  vec4 direction;\n"
        "  vec4 eye;\n"
        "  float ambientIntensity;\n"        
        "  float diffuseIntensity;\n"
        "  float specularIntensity;\n"
        "  float specularPower;\n"        
        "} uData;\n"

        "void main() {\n"
        "  gl_Position = uData.mvp * vec4(position, 1.0);\n"        
        "  vTexCoord = texcoord;\n"
        "  vNormal = mat3(uData.normal) * normal;\n"
        "  vWorldPos = (uData.world * vec4(position, 1.0)).xyz;\n"
        "}";

    const std::string FRAGMENT_SHADER =
        "#version 450\n"
        "layout (location = 0) in vec2 vTexCoord;\n"
        "layout (location = 1) in vec3 vNormal;\n"
        "layout (location = 2) in vec3 vWorldPos;\n"
        "layout (location = 0) out vec4 fColor;\n"

        "uniform sampler2D uImage;\n"

        "layout (binding = 0, std140) uniform Data {\n"
        "  mat4 mvp;\n"
        "  mat4 normal;\n"
        "  mat4 world;\n"
        "  vec4 color;\n"
        "  vec4 direction;\n"
        "  vec4 eye;\n"
        "  float ambientIntensity;\n"        
        "  float diffuseIntensity;\n"
        "  float specularIntensity;\n"
        "  float specularPower;\n"        
        "} uData;\n"

        "void main() {\n"
        "  vec4 ambientColor = vec4(uData.color.rgb * uData.ambientIntensity, 1.0);\n"
        "  vec3 lightDirection = -uData.direction.xyz;\n"
        "  vec3 normal = normalize(vNormal);\n"    
        "  float diffuseFactor = dot(normal, lightDirection);\n"
        "  vec4 diffuseColor = vec4(0.0);\n"
        "  vec4 specularColor = vec4(0.0);\n"

        "  if (diffuseFactor > 0.0) {\n"
        "    diffuseColor = vec4(uData.color.rgb * uData.diffuseIntensity * diffuseFactor, 1.0);\n"

        "    vec3 vertexToEye = normalize(uData.eye.xyz - vWorldPos);\n"
        "    vec3 lightReflect = normalize(reflect(uData.direction.xyz, normal));\n"
        "    float specularFactor = dot(vertexToEye, lightReflect);\n"

        "    if (specularFactor > 0.0) {\n"
        "      specularFactor = pow(specularFactor, uData.specularPower);\n"
        "      specularColor = vec4(uData.color.rgb * uData.specularIntensity * specularFactor, 1.0);\n"
        "    }\n"
        "  } else {\n"
        "    diffuseColor = vec4(0.0);\n"
        "  }\n"
        "  fColor = texture(uImage, vTexCoord) * (ambientColor + diffuseColor + specularColor);\n"        
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

    auto window = glfwCreateWindow(640, 480, "Tutorial19", nullptr, nullptr);

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

    struct __attribute__ ((packed)) Vertex {
        float x, y, z;
        float u, v;
        float nx, ny, nz;
    };

    auto points = std::vector<Vertex> ();
    points.push_back({ -1.0F, -1.0F, 0.5773F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F });
    points.push_back({ 0.0F, -1.0F, -1.15475F, 0.5F, 0.0F, 0.0F, 0.0F, 0.0F });
    points.push_back({ 1.0F, -1.0F, 0.5773F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F });
    points.push_back({ 0.0F, 1.0F, 0.0F, 0.5F, 1.0F, 0.0F, 0.0F, 0.0F });

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

        auto v1 = glm::vec3(p1.x, p1.y, p1.z) - glm::vec3(p0.x, p0.y, p0.z);
        auto v2 = glm::vec3(p2.x, p2.y, p2.z) - glm::vec3(p0.x, p0.y, p0.z);
        auto normal = glm::normalize(glm::cross(v1, v2));
        
        p0.nx += normal.x;
        p0.ny += normal.y;
        p0.nz += normal.z;

        p1.nx += normal.x;
        p1.ny += normal.y;
        p1.nz += normal.z;

        p2.nx += normal.x;
        p2.ny += normal.y;
        p2.nz += normal.z;
    }

    for (auto& p : points) {
        auto n = glm::normalize(glm::vec3(p.nx, p.ny, p.nz));

        p.nx = n.x;
        p.ny = n.y;
        p.nz = n.z;
    }

    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW);

    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glNamedBufferData(ibo, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    struct UData {
        glm::mat4 mvp;
        glm::mat4 normal;
        glm::mat4 world;
        glm::vec4 color;
        glm::vec4 direction;
        glm::vec4 eye;
        glm::float32 ambientIntensity;        
        glm::float32 diffuseIntensity;        
        glm::float32 specularIntensity;
        glm::float32 specularPower;
    };

    GLuint ubo;
    glCreateBuffers(1, &ubo);
    glNamedBufferStorage(ubo, sizeof(UData), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    auto pData = reinterpret_cast<UData * > (glMapNamedBufferRange(ubo, 0, sizeof(UData), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

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
    auto uData = glGetUniformBlockIndex(program, "Data");

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
        auto trNormal = glm::transpose(glm::inverse(trMv));        

        pData->mvp = trProj * trMv;
        pData->normal = glm::transpose(glm::inverse(trMv));
        pData->world = trMv;
        pData->color = glm::vec4(1.0F);
        pData->direction = glm::vec4(1.0F, 0.0F, 0.0F, 1.0F);
        pData->ambientIntensity = userData.ambientIntensity;
        pData->diffuseIntensity = 0.25F;
        pData->specularIntensity = 1.0F;
        pData->specularPower = 32.0F;
        pData->eye = glm::vec4(userData.pCamera->getPosition(), 1.0);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(program);        
        glUniform1i(uImage, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, uData, ubo);

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
