#pragma once

#include <glad/glad.h>

template<typename UniformType>
class UniformBuffer  {
public:
    UniformBuffer() {
        InitRemap visitor(_uniformRemap);
        static_cast<UniformType*>(nullptr)->foreach_member(visitor);
    }

    void InitGLLocations(GLuint program) {
        for (NameLocation& r : _uniformRemap) {
            r.location = glGetUniformLocation(program, r.name.c_str());
        }
    }

    void Set(const UniformType& uniform) {
        SetUniform visitor(_uniformRemap);
        uniform.foreach_member(visitor);
    }

private:
    struct NameLocation {
        std::string name;
        GLint       location;
    };

    struct InitRemap {
        InitRemap(std::vector<NameLocation>& uniformRemap)
            : _uniformRemap(uniformRemap)
        {}
        template<typename field_t>
        void operator() (const char* name, const field_t& field) {
            NameLocation info = {};
            info.name = name;
            info.location = -1;

            _uniformRemap.push_back(info);
        }
    private:
        std::vector<NameLocation>& _uniformRemap;
    };

    struct SetUniform {
        SetUniform(std::vector<NameLocation>& uniformRemap)
            : _remap(uniformRemap)
            , _index(0u)
        {}

        void operator()(const char*, const float& val)     { glUniform1f(_remap[_index].location, val);                                   ++_index; }
        void operator()(const char*, const glm::vec2& vec) { glUniform2f(_remap[_index].location, vec.x, vec.y);                          ++_index; }
        void operator()(const char*, const glm::vec3& vec) { glUniform3f(_remap[_index].location, vec.x, vec.y, vec.z);                   ++_index; }
        void operator()(const char*, const glm::vec4& vec) { glUniform4f(_remap[_index].location, vec.x, vec.y, vec.z, vec.w);            ++_index; }
        void operator()(const char*, const glm::mat2& m)   { glUniformMatrix2fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        void operator()(const char*, const glm::mat3& m)   { glUniformMatrix3fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        void operator()(const char*, const glm::mat4& m)   { glUniformMatrix4fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
    private:
        std::vector<NameLocation>& _remap;
        GLuint                     _index;
    };

    std::vector<NameLocation> _uniformRemap;
};
