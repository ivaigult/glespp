#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

template<typename CType>
struct gl_type_traits;

#define REGISTER_GL_TYPE(c_type, gl_type, gl_size) \
    template<>                                     \
    struct gl_type_traits<c_type>                  \
    {                                              \
        static constexpr GLint   type   = gl_type; \
        static constexpr GLsizei size = gl_size;   \
    }

REGISTER_GL_TYPE(uint16_t, GL_UNSIGNED_SHORT, 1);
REGISTER_GL_TYPE(uint32_t, GL_UNSIGNED_INT, 1);
REGISTER_GL_TYPE(int16_t, GL_SHORT, 1);
REGISTER_GL_TYPE(int32_t, GL_INT, 1);

REGISTER_GL_TYPE(float, GL_FLOAT, 1);



REGISTER_GL_TYPE(glm::vec1, GL_FLOAT, 1);
REGISTER_GL_TYPE(glm::vec2, GL_FLOAT, 2);
REGISTER_GL_TYPE(glm::vec3, GL_FLOAT, 3);
REGISTER_GL_TYPE(glm::vec4, GL_FLOAT, 4);
#undef REGISTER_GL_TYPE