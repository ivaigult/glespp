#pragma once

#include <glad/glad.h>

#include <texture.hpp>

namespace glespp {
namespace detail {

template<typename uniform_t>
class unibofrm_buffer  {
public:
    typedef uniform_t uniform_type;

    unibofrm_buffer() {
        init_remap visitor(_uniformRemap);
        static_cast<uniform_type*>(nullptr)->foreach_member(visitor);
    }

    void post_link(GLuint program) {
        for (name_location& r : _uniformRemap) {
            r.location = glGetUniformLocation(program, r.name.c_str());
        }
    }

    void set(const uniform_type& uniform) {
        set_uniform visitor(_uniformRemap);
        uniform.foreach_member(visitor);
    }

private:
    struct name_location {
        std::string name;
        GLint       location;
    };

    struct init_remap {
        init_remap(std::vector<name_location>& uniformRemap)
            : _uniformRemap(uniformRemap)
        {}
        template<typename field_t>
        void operator() (const char* name, const field_t& field) {
            name_location info = {};
            info.name     = name;
            info.location = -1;

            _uniformRemap.push_back(info);
        }
    private:
        std::vector<name_location>& _uniformRemap;
    };

    struct set_uniform {
        set_uniform(std::vector<name_location>& uniformRemap)
            : _remap(uniformRemap)
            , _index(0u)
            , _tex_slot(0)
        {}

        void operator()(const char*, const float& val)     { glUniform1f(_remap[_index].location, val);                                   ++_index; }
        void operator()(const char*, const glm::vec2& vec) { glUniform2f(_remap[_index].location, vec.x, vec.y);                          ++_index; }
        void operator()(const char*, const glm::vec3& vec) { glUniform3f(_remap[_index].location, vec.x, vec.y, vec.z);                   ++_index; }
        void operator()(const char*, const glm::vec4& vec) { glUniform4f(_remap[_index].location, vec.x, vec.y, vec.z, vec.w);            ++_index; }
        void operator()(const char*, const glm::mat2& m)   { glUniformMatrix2fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        void operator()(const char*, const glm::mat3& m)   { glUniformMatrix3fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        void operator()(const char*, const glm::mat4& m)   { glUniformMatrix4fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        
        void operator()(const char*, const glespp::texture_ref& tr) {
            GLenum tex_slot_enum = GL_TEXTURE0 + _tex_slot;
            glActiveTexture(tex_slot_enum);
            glBindTexture(tr.target, tr.id);
            glUniform1i(_remap[_index].location, _tex_slot);
            ++_tex_slot;
        }
    private:
        std::vector<name_location>& _remap;
        GLuint                      _index;
        GLuint                      _tex_slot;
    };

    std::vector<name_location> _uniformRemap;
};

} // detail
} // glespp
