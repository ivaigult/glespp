#pragma once

#include <glad/glad.h>

#include <texture.hpp>

namespace glespp {
namespace detail {

template<typename reflectable_t, typename visitor_t>
struct has_foreach_member {
    template
    <
        typename some_reflectable_t, 
        typename some_visitor_t,
        void (some_reflectable_t::*)(some_visitor_t& v) const
    > 
    struct sfinae;

    template<typename some_reflectable_t, typename some_visitor_t>
    static std::true_type test(sfinae<some_reflectable_t, some_visitor_t, &some_reflectable_t::foreach_member>*);
    template<typename some_reflectable_t, typename some_visitor_t>
    static std::false_type test(...);

    typedef decltype(test<reflectable_t, visitor_t>(nullptr)) type;
};

template<typename uniform_t>
class uniform_buffer  {
public:
    typedef uniform_t uniform_type;

    uniform_buffer() {
        init_remap_visitor visitor(_uniformRemap);
        static_cast<uniform_type*>(nullptr)->foreach_member(visitor);
    }

    void post_link(GLuint program) {
        for (name_location& r : _uniformRemap) {
            r.location = glGetUniformLocation(program, r.name.c_str());
        }
    }

    void set(const uniform_type& uniform) {
        set_uniform_visitor visitor(_uniformRemap);
        uniform.foreach_member(visitor);
    }

private:
    struct name_location {
        std::string name;
        GLint       location;
    };

    struct init_remap_visitor {
        init_remap_visitor(std::vector<name_location>& uniformRemap, std::string base_name = "")
            : _uniformRemap(uniformRemap)
            , _base_name(base_name)
        {}

        template<typename field_t>
        void operator() (const char* name, const field_t& field)
        {
            typedef typename has_foreach_member<field_t, init_remap_visitor>::type has_foreach_t;
            (*this)(has_foreach_t{}, name, field);
        }

        template<typename field_t>
        void operator() (std::true_type, const char* name, const field_t& field) {
            init_remap_visitor visitor(_uniformRemap, _base_name + name + ".");
            field.foreach_member(visitor);
        }

        template<typename field_t>
        void operator() (std::false_type, const char* name, const field_t& field) {
            name_location info = {};
            info.name          = _base_name + name;
            info.location      = -1;

            _uniformRemap.push_back(info);
        }
    private:
        std::vector<name_location>& _uniformRemap;
        std::string                 _base_name;
    };

    struct set_uniform_visitor {
        set_uniform_visitor(std::vector<name_location>& uniformRemap)
            : _remap(uniformRemap)
            , _index(0u)
            , _tex_slot(0)
        {}

        template<typename field_t>
        void operator() (const char* name, const field_t& field)
        {
            typedef typename has_foreach_member<field_t, init_remap_visitor>::type has_foreach_t;
            (*this)(has_foreach_t{}, name, field);
        }

        template<typename field_t>
        void operator() (std::true_type, const char*, const field_t& field) {
            set_uniform_visitor visitor(_remap);
            visitor._index = _index;
            visitor._tex_slot = _tex_slot;
            
            field.foreach_member(visitor);

            _index = visitor._index;
            _tex_slot = visitor._tex_slot;
        }

        void operator()(std::false_type, const char*, const float& val)     { glUniform1f(_remap[_index].location, val);                                   ++_index; }
        void operator()(std::false_type, const char*, const glm::vec2& vec) { glUniform2f(_remap[_index].location, vec.x, vec.y);                          ++_index; }
        void operator()(std::false_type, const char*, const glm::vec3& vec) { glUniform3f(_remap[_index].location, vec.x, vec.y, vec.z);                   ++_index; }
        void operator()(std::false_type, const char*, const glm::vec4& vec) { glUniform4f(_remap[_index].location, vec.x, vec.y, vec.z, vec.w);            ++_index; }
        void operator()(std::false_type, const char*, const glm::mat2& m)   { glUniformMatrix2fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        void operator()(std::false_type, const char*, const glm::mat3& m)   { glUniformMatrix3fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        void operator()(std::false_type, const char*, const glm::mat4& m)   { glUniformMatrix4fv(_remap[_index].location, 1, GL_FALSE, glm::value_ptr(m)); ++_index; }
        
        void operator()(std::false_type, const char*, const glespp::texture_ref& tr) {
            GLenum tex_slot_enum = GL_TEXTURE0 + _tex_slot;
            glActiveTexture(tex_slot_enum);
            glBindTexture(tr.target, tr.id);
            glUniform1i(_remap[_index].location, _tex_slot);
            ++_tex_slot;
            ++_index;
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
