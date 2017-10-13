#pragma once

#include <glad/glad.h>

#include "detail/type_traits.hpp"
#include "detail/va_object.hpp"
#include "detail/ub_object.hpp"
#include "vb_object.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <type_traits>

#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glespp {
namespace detail {
class shader {
public:
    enum Type {
        vertex,
        fragment,
    };

    shader(std::istream& is, Type t) {
        _source.reserve(1 << 12);

        GLenum shader_type = GL_VERTEX_SHADER;
        switch (t) {
        case vertex:
            shader_type = GL_VERTEX_SHADER;
            break;
        case fragment:
            shader_type = GL_FRAGMENT_SHADER;
            break;
        }

        _id = glCreateShader(shader_type);
        if (!_id) {
            throw std::runtime_error("Failed to create shader");
        }

        _source.assign((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        _source.push_back('\0');

        char* shader_text = _source.data();
        glShaderSource(_id, 1, &shader_text, nullptr);
        glCompileShader(_id);

        GLint status = GL_FALSE;
        glGetShaderiv(_id, GL_COMPILE_STATUS, &status);
        if (GL_FALSE == status) {
            std::vector<char> info_log;
            GLint info_log_len = 0;
            glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &info_log_len);
            info_log.resize(info_log_len + 1);

            glGetShaderInfoLog(_id, (GLsizei)info_log.size(), nullptr, info_log.data());
            OutputDebugStringA(info_log.data());
            throw std::runtime_error(std::string("Compilation failed: \n") + info_log.data());
        }
    }

    ~shader() {
        glDeleteShader(_id);
    }

    GLuint get_id() const {
        return _id;
    }
private:
    GLuint            _id;
    std::vector<char> _source;
};
} // detail

enum class geom_topology {
    points,
    line_strip,
    line_loop,
    lines,
    triangle_strip,
    triangle_fan,
    triangles,
};

inline GLenum geom_topology2gl_enum(geom_topology t) {
    switch (t) {
    case geom_topology::points:         return GL_POINTS;
    case geom_topology::line_strip:     return GL_LINE_STRIP;
    case geom_topology::line_loop:      return GL_LINE_LOOP;
    case geom_topology::lines:          return GL_LINES;
    case geom_topology::triangle_strip: return GL_TRIANGLE_STRIP;
    case geom_topology::triangle_fan:   return GL_TRIANGLE_FAN;
    case geom_topology::triangles:      return GL_TRIANGLES;
    default:                            return GL_TRIANGLES;
    }
}

template<typename vertex_t, typename uniform_t>
class program {
public:
    typedef vertex_t  vertex_type;
    typedef uniform_t uniform_type;

    program(std::istream& vSource, std::istream& fSource)
        : _id(0)
        , _uniform({})
        , _attribs(0u)
    {
        detail::shader vertex_sh(vSource, detail::shader::vertex);
        detail::shader fragment_sh(fSource, detail::shader::fragment);

        _id = glCreateProgram();
        glAttachShader(_id, vertex_sh.get_id());
        glAttachShader(_id, fragment_sh.get_id());

        _vao.pre_link(_id);
        glLinkProgram(_id);
        _vao.post_link(_id);
        _ubo.post_link(_id);

        glDetachShader(_id, vertex_sh.get_id());
        glDetachShader(_id, fragment_sh.get_id());

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

    ~program() {
        glDeleteProgram(_id);
    }

    void set_uniform(const uniform_type& u)                    { _uniform = u;               }
    void set_attribs(const buffer_object<vertex_type>& buffer) { _attribs = buffer.get_id(); }

    template<typename index_t>
    void execute(geom_topology t, buffer_object<index_t>& indices, size_t start, size_t count) {
        glUseProgram(_id);
        _ubo.set(_uniform);
        glBindBuffer(GL_ARRAY_BUFFER, _attribs);
        detail::vao_guard<vertex_type> guard(_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.get_id());

        GLenum mode = geom_topology2gl_enum(t);
        GLenum type = gl_type_traits<index_t>::type;
        glDrawElements(
            mode, 
            static_cast<GLsizei>(indices.size() - start), 
            type, 
            reinterpret_cast<void*>(sizeof(index_t)*start)
        );
    }

    template<typename index_t>
    void execute(geom_topology t, buffer_object<index_t>& indices) {
        execute(t, indices, 0, indices.size());
    }

    void execute(geom_topology t, size_t start, size_t size) {
        glUseProgram(_id);
        _ubo.set(_uniform);
        glBindBuffer(GL_ARRAY_BUFFER, _attribs);
        detail::vao_guard<vertex_type> guard(_vao);

        GLenum mode = geom_topology2gl_enum(t);
        glDrawArrays(mode, (GLint)start, (GLsizei)size);
    }

private:
    GLuint                                _id;
    uniform_type                          _uniform;
    GLuint                                _attribs; // TODO: what if buffer is removed?
    detail::vertex_array<vertex_type>     _vao;
    detail::unibofrm_buffer<uniform_type> _ubo;
};

} // glespp
