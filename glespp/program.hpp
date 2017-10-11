#pragma once

#include <glad/glad.h>

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <type_traits>

#include <cassert>

#include "detail/type_traits.hpp"
#include "detail/va_object.hpp"
#include "detail/ub_object.hpp"
#include "vb_object.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    enum Type {
        kVertex,
        kFragment,
    };

    Shader(std::istream& is, Type t) {
        _source.reserve(1 << 12);

        GLenum shaderType = GL_VERTEX_SHADER;
        switch (t) {
        case kVertex:
            shaderType = GL_VERTEX_SHADER;
            break;
        case kFragment:
            shaderType = GL_FRAGMENT_SHADER;
            break;
        }

        _id = glCreateShader(shaderType);
        if (!_id) {
            throw std::runtime_error("Failed to create shader");
        }

        _source.assign((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        _source.push_back('\0');

        char* shaderText = _source.data();
        glShaderSource(_id, 1, &shaderText, nullptr);
        glCompileShader(_id);

        GLint status = GL_FALSE;
        glGetShaderiv(_id, GL_COMPILE_STATUS, &status);
        if (GL_FALSE == status) {
            std::vector<char> infoLog;
            GLint infologLen = 0;
            glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &infologLen);
            infoLog.resize(infologLen + 1);

            glGetShaderInfoLog(_id, (GLsizei)infoLog.size(), nullptr, infoLog.data());
            throw std::runtime_error(std::string("Compilation failed: \n") + infoLog.data());
        }
    }

    ~Shader() {
        glDeleteShader(_id);
    }

    GLuint GetId() const {
        return _id;
    }
private:
    GLuint            _id;
    std::vector<char> _source;
};

enum class GeomTopology {
    Points,
    LineStrip,
    LineLoop,
    Lines,
    TriangleStrip,
    TriangleFan,
    Triangles,
};

inline GLenum GeomTopology2GLEnum(GeomTopology t) {
    switch (t) {
    case GeomTopology::Points:        return GL_POINTS;
    case GeomTopology::LineStrip:     return GL_LINE_STRIP;
    case GeomTopology::LineLoop:      return GL_LINE_LOOP;
    case GeomTopology::Lines:         return GL_LINES;
    case GeomTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
    case GeomTopology::TriangleFan:   return GL_TRIANGLE_FAN;
    case GeomTopology::Triangles:     return GL_TRIANGLES;
    default:                          return GL_TRIANGLES;
    }
}

template<typename VertexType, typename UniformType>
class Program {
public:
    Program(std::istream& vSource, std::istream& fSource)
        : _id(0)
        , _uniform({})
        , _attribs(0u)
    {
        Shader vSh(vSource, Shader::kVertex);
        Shader fSh(fSource, Shader::kFragment);

        _id = glCreateProgram();
        glAttachShader(_id, vSh.GetId());
        glAttachShader(_id, fSh.GetId());

        _vao.BindAttribLocations(_id);
        glLinkProgram(_id);
        _vao.Minify(_id);
        _ubo.InitGLLocations(_id);

        glDetachShader(_id, vSh.GetId());
        glDetachShader(_id, fSh.GetId());

        GLint status = GL_FALSE;
        glGetProgramiv(_id, GL_LINK_STATUS, &status);

        if (GL_FALSE == status) {
            std::vector<char> infoLog;
            GLint infologLen = 0;
            glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &infologLen);
            infoLog.resize(infologLen + 1);

            glGetProgramInfoLog(_id, (GLsizei)infoLog.size(), nullptr, infoLog.data());
            throw std::runtime_error(std::string("Link failed: \n") + infoLog.data());
        }
    }

    ~Program() {
        glDeleteProgram(_id);
    }

    void SetUniform(const UniformType& u)             { _uniform = u;              }
    void SetAttribs(const Buffer<VertexType>& buffer) { _attribs = buffer.GetId(); }

    template<typename IndexType>
    void Execute(GeomTopology t, Buffer<IndexType>& indices, size_t start, size_t count) {
        glUseProgram(_id);
        _ubo.Set(_uniform);
        glBindBuffer(GL_ARRAY_BUFFER, _attribs);
        VAOGuard<VertexType> guard(_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.GetId());

        GLenum mode = GeomTopology2GLEnum(t);
        GLenum type = GLTypeTraits<IndexType>::type;
        glDrawElements(mode, (GLsizei)size, type, (void*) sizeof(IndexType)*start);
    }

    void Execute(GeomTopology t, size_t start, size_t size) {
        glUseProgram(_id);
        _ubo.Set(_uniform);
        glBindBuffer(GL_ARRAY_BUFFER, _attribs);
        VAOGuard<VertexType> guard(_vao);

        GLenum mode = GeomTopology2GLEnum(t);
        glDrawArrays(mode, (GLint)start, (GLsizei)size);
    }

private:
    GLuint                         _id;
    UniformType                    _uniform;
    GLuint                         _attribs; // TODO: what if buffer is removed?
    VertexArray<VertexType>        _vao;
    UniformBuffer<UniformType>     _ubo;
};

