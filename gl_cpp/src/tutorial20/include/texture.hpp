#pragma once

#include <GL/glew.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "stb_image.hpp"

namespace gfx {
    class Texture {
        GLuint _handle;
        GLenum _target;    

        Texture(const Texture&) = delete;

        Texture& operator= (const Texture&) = delete;

    public:
        Texture(GLenum target, const std::string& fileName);

        Texture(Texture&& other) noexcept;

        ~Texture() noexcept;

        Texture& operator= (Texture&& other) noexcept;

        void bind(GLuint unit) noexcept;
    };

    Texture::Texture(GLenum target, const std::string& fileName) {
        auto file = std::ifstream(fileName.c_str(), std::ios::binary | std::ios::ate);
        auto size = file.tellg();
    
        file.seekg(0, std::ios::beg);

        auto buffer = std::make_unique<char[]> (size);

        if (!file.read(buffer.get(), size)) {
            auto msg = std::stringstream();
            msg << "Failed to load file: \"" << fileName << "\"";

            throw std::runtime_error(msg.str());
        }

        int x, y, channels;
        auto mem = stbi_load_from_memory(reinterpret_cast<stbi_uc * > (buffer.get()), size, &x, &y, &channels, 4);

        buffer = nullptr;

        glCreateTextures(target, 1, &_handle);
        glTextureStorage2D(_handle, 1, GL_RGBA8, static_cast<GLsizei> (x), static_cast<GLsizei> (y));
        glTextureSubImage2D(_handle, 0, 0, 0, static_cast<GLsizei> (x), static_cast<GLsizei> (y), GL_RGBA, GL_UNSIGNED_BYTE, mem);

        stbi_image_free(mem);
    }

    Texture::Texture(Texture&& other) noexcept {
        _handle = other._handle;
        _target = other._target;

        other._handle = 0;
        other._target = 0;
    }

    Texture::~Texture() noexcept {
        if (_handle) {
            glDeleteTextures(1, &_handle);
        }
    }

    Texture& Texture::operator= (Texture&& other) noexcept {
        std::swap(other._handle, _handle);
        std::swap(other._target, _target);

        return *this;
    }

    void Texture::bind(GLuint unit) noexcept {
        glBindTextureUnit(unit, _handle);
    }
}
