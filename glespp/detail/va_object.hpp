#pragma once

#include <algorithm>
#include <vector>
#include <set>
#include <string>

#include <glad/glad.h>

#include "type_traits.hpp"

namespace glespp {
namespace detail {
template<typename vertex_t>
class vertex_array {
public:
    typedef vertex_t vertex_type;
    vertex_array() {
        attrib_enumerator attribEnumerator(_vaps);
        static_cast<vertex_type*>(nullptr)->foreach_member(attribEnumerator);
    }

    void pre_link(GLuint program) {
        GLuint index = 0;
        for (const vap_args& args : _vaps) {
            glBindAttribLocation(program, index, args.name.c_str());
            ++index;
        }
    }
    void post_link(GLuint program) {
        GLint num_attribs = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &num_attribs);

        GLint max_name_len = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_name_len);
        std::vector<char> cur_name(max_name_len + 1);

        std::set<GLint> active_indices;

        for (GLint i = 0; i < num_attribs; ++i) {
            glGetActiveAttrib(program, i, (GLsizei)cur_name.size(), nullptr, nullptr, nullptr, cur_name.data());
            GLint index = glGetAttribLocation(program, cur_name.data());
            active_indices.insert(index);
        }

        typename std::vector<vap_args>::iterator result = std::remove_if(
            _vaps.begin(),
            _vaps.end(),
            [&active_indices](vap_args& args) -> bool {
            return !active_indices.count(args.index);
        });

        _vaps.erase(result, _vaps.end());
    }
    void bind() {
        for (const vap_args& args : _vaps) {
            glVertexAttribPointer(args.index, args.size, args.type, args.normalized, args.stride, args.pointer);
            glEnableVertexAttribArray(args.index);
        }
    }
    void unblind() {
        for (const vap_args& args : _vaps) {
            glDisableVertexAttribArray(args.index);
        }
    }
private:
    struct vap_args {
        std::string    name;
        GLuint         index;
        GLint          size;
        GLenum         type;
        GLboolean      normalized;
        GLsizei        stride;
        const GLvoid * pointer;
    };

    class attrib_enumerator {
    public:
        attrib_enumerator(std::vector<vap_args>& vaps)
            : _index(0u)
            , _vaps(vaps)
        {}

        template<typename field_t>
        void operator()(const char* name, const field_t& field) {
            size_t offset = reinterpret_cast<size_t>(&field);

            vap_args args   = {};
            args.name       = name;
            args.index      = _index;
            args.size       = gl_type_traits<field_t>::size;
            args.type       = gl_type_traits<field_t>::type;
            args.normalized = GL_FALSE;
            args.stride     = sizeof(vertex_type);
            args.pointer    = (const void*) offset;

            _vaps.push_back(args);
            ++_index;
        }
    private:
        GLuint                 _index;
        std::vector<vap_args>& _vaps;
    };

    std::vector<vap_args> _vaps;
};

template<typename vertex_t>
struct vao_guard {
    vao_guard(vertex_array<vertex_t>& vao)
        : _vao(vao)
    { _vao.bind(); }

    ~vao_guard()
    { _vao.unblind(); }

    vertex_array<vertex_t>& _vao;
};

} // detail
} // glespp
