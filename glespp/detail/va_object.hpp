#pragma once

#include <algorithm>
#include <vector>
#include <set>

#include <glad/glad.h>

#include "type_traits.hpp"

template<typename VertexType>
class VertexArray {
public:
    VertexArray() {
        AttribEnumerator attribEnumerator(_vaps);
        static_cast<VertexType*>(nullptr)->foreach_member(attribEnumerator);
    }
    virtual void BindAttribLocations(GLuint program) {
        GLuint index = 0;
        for (const VAPArgs& args : _vaps) {
            glBindAttribLocation(program, index, args.name.c_str());
            ++index;
        }
    }
    void Minify(GLuint program) {
        GLint numAttribs = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttribs);

        GLint maxNameLen = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLen);
        std::vector<char> curName(maxNameLen + 1);

        std::set<GLint> activeIndices;

        for (GLint i = 0; i < numAttribs; ++i) {
            glGetActiveAttrib(program, i, (GLsizei)curName.size(), nullptr, nullptr, nullptr, curName.data());
            GLint index = glGetAttribLocation(program, curName.data());
            activeIndices.insert(index);
        }

        std::vector<VAPArgs>::iterator result = std::remove_if(
            _vaps.begin(),
            _vaps.end(),
            [&activeIndices](VAPArgs& args) -> bool {
            return !activeIndices.count(args.index);
        });

        _vaps.erase(result, _vaps.end());
    }
    void Bind() {
        for (const VAPArgs& args : _vaps) {
            glVertexAttribPointer(args.index, args.size, args.type, args.normalized, args.stride, args.pointer);
            glEnableVertexAttribArray(args.index);
        }
    }
    void Unbind() {
        for (const VAPArgs& args : _vaps) {
            glDisableVertexAttribArray(args.index);
        }
    }
private:
    struct VAPArgs {
        std::string    name;
        GLuint         index;
        GLint          size;
        GLenum         type;
        GLboolean      normalized;
        GLsizei        stride;
        const GLvoid * pointer;
    };

    class AttribEnumerator {
    public:
        AttribEnumerator(std::vector<VAPArgs>& vaps)
            : _index(0u)
            , _vaps(vaps)
        {}

        template<typename field_t>
        void operator()(const char* name, const field_t& field) {
            size_t offset = reinterpret_cast<size_t>(&field);

            VAPArgs args = {};
            args.name       = name;
            args.index      = _index;
            args.size       = GLTypeTraits<field_t>::size;
            args.type       = GLTypeTraits<field_t>::type;
            args.normalized = GL_FALSE;
            args.stride     = sizeof(VertexType);
            args.pointer    = (const void*) offset;

            _vaps.push_back(args);
            ++_index;
        }
    private:
        GLuint                _index;
        std::vector<VAPArgs>& _vaps;
    };

    std::vector<VAPArgs> _vaps;
};

template<typename VertexType>
struct VAOGuard {
    VAOGuard(VertexArray<VertexType>& vao)
        : _vao(vao)
    { _vao.Bind(); }

    ~VAOGuard()
    { _vao.Unbind(); }

    VertexArray<VertexType>& _vao;
};