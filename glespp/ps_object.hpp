#pragma once

#include <glad/glad.h>

#include "reflection.hpp"

#define PP_IMPL_ENUM_VALUE(cookie, enum_pair) __PP_EVAL(__PP_IMPL_ENUM_VALUE enum_pair) 
#define __PP_IMPL_ENUM_VALUE(cpp_enum, gl_enum) cpp_enum,

#define PP_IMPL_CONVERT(class_name, enum_pair) __PP_EVAL(__PP_IMPL_CONVERT PP_CONS(class_name, enum_pair)) 
#define __PP_IMPL_CONVERT(class_name, cpp_enum, gl_enum) case class_name::cpp_enum: return gl_enum;

#define DEF_STATE_ENUM(name, ...)                       \
    enum class name {                                   \
        PP_FOR_EACH(PP_IMPL_ENUM_VALUE, _, ##__VA_ARGS__) \
    };                                                  \
    namespace detail {                                  \
    GLenum name ## 2gl(name e) {                        \
        switch(e) {                                     \
            PP_FOR_EACH(PP_IMPL_CONVERT, name, ##__VA_ARGS__) \
            default:                                    \
                assert(!"Unknown enum");                \
                return GL_NONE;                         \
        }                                               \
        return GL_NONE;                                 \
    }                                                   \
    } // detail

namespace glespp {
DEF_STATE_ENUM(depth_func,
    (never,     GL_NEVER   ),
    (less,      GL_LESS    ),
    (equal,     GL_EQUAL   ),
    (lequal,    GL_LEQUAL  ),
    (greater,   GL_GREATER ),
    (not_equal, GL_NOTEQUAL),
    (gequal,    GL_GEQUAL  ),
    (always,    GL_ALWAYS  )
);

}