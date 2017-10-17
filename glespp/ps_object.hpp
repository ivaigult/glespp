#pragma once

#include <glad/glad.h>

#include "reflection.hpp"

#define PP_IMPL_ENUM_VALUE(cookie, enum_pair) __PP_EVAL(__PP_IMPL_ENUM_VALUE enum_pair) 
#define __PP_IMPL_ENUM_VALUE(cpp_enum, gl_enum) cpp_enum,

#define PP_IMPL_CONVERT(class_name, enum_pair) __PP_EVAL(__PP_IMPL_CONVERT PP_CONS(class_name, enum_pair)) 
#define __PP_IMPL_CONVERT(class_name, cpp_enum, gl_enum) case class_name::cpp_enum: return gl_enum;

#define DEF_STATE_ENUM(name, ...)                             \
    enum class name {                                         \
        PP_FOR_EACH(PP_IMPL_ENUM_VALUE, _, ##__VA_ARGS__)     \
    };                                                        \
    namespace detail {                                        \
    GLenum name ## 2gl(name e) {                              \
        switch(e) {                                           \
            PP_FOR_EACH(PP_IMPL_CONVERT, name, ##__VA_ARGS__) \
            default:                                          \
                assert(!"Unknown enum");                      \
                return GL_NONE;                               \
        }                                                     \
    }                                                         \
    } // detail

#define PP_IMPL_STATE_MEMBER(cookie, type_name_apply) __PP_EVAL(__PP_IMPL_STATE_MEMBER type_name_apply)
#define __PP_IMPL_STATE_MEMBER(type, name, apply) type name;

#define PP_IMPL_STATE_APPLY(cookie, type_name_apply) __PP_EVAL(__PP_IMPL_STATE_APPLY type_name_apply)
#define __PP_IMPL_STATE_APPLY(type, name, apply) apply;

#define DEF_STATE_GROUP(name, ...)                              \
    struct name {                                               \
        PP_FOR_EACH(PP_IMPL_STATE_MEMBER, _, ##__VA_ARGS__);    \
        void apply() const {                                    \
            PP_FOR_EACH(PP_IMPL_STATE_APPLY, _, ##__VA_ARGS__); \
        }                                                       \
    }

namespace glespp {
namespace detail {
static inline void glSwitch(GLenum cap, GLint enabled) {
    if (enabled) {
        glEnable(cap);
    } else {
        glDisable(cap);
    }
}
}

DEF_STATE_ENUM(boolean,
    (on,  GL_TRUE),
    (off, GL_FALSE)
);

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

DEF_STATE_ENUM(blend_equation,
    (add,              GL_FUNC_ADD), 
    (subtract,         GL_FUNC_SUBTRACT),
    (reverse_subtract, GL_FUNC_REVERSE_SUBTRACT)
);

DEF_STATE_ENUM(blend_function,
    (zero,                     GL_ZERO),
    (one,                      GL_ONE),
    (src_color,                GL_SRC_COLOR),
    (one_minus_src_color,      GL_ONE_MINUS_SRC_COLOR),
    (dst_color,                GL_DST_COLOR),
    (one_minus_dst_color,      GL_ONE_MINUS_DST_COLOR),
    (src_alpha,                GL_SRC_ALPHA),
    (one_minus_src_alpha,      GL_ONE_MINUS_SRC_ALPHA),
    (dst_alpha,                GL_DST_ALPHA),
    (one_minus_dst_alpha,      GL_ONE_MINUS_DST_ALPHA),
    (constant_color,           GL_CONSTANT_COLOR),
    (one_minus_constant_color, GL_ONE_MINUS_CONSTANT_COLOR),
    (constant_alpha,           GL_CONSTANT_ALPHA),
    (one_minus_constant_alpha, GL_ONE_MINUS_CONSTANT_ALPHA),
    (src_alpha_saturate,       GL_SRC_ALPHA_SATURATE)
)

struct blend_equation_separate {
    blend_equation rgb;
    blend_equation alpha;
    bool operator==(const blend_equation_separate& that) const { return rgb == that.rgb && alpha == that.alpha; }
    void apply() const { glBlendEquationSeparate(detail::blend_equation2gl(rgb), detail::blend_equation2gl(alpha)); }
};

struct blend_function_separate {
    blend_function src_rgb;
    blend_function dst_rgb;
    blend_function src_alpha;
    blend_function dst_alpha;
    bool operator==(const blend_function_separate& that) const {
        return src_rgb   == that.src_rgb   &&
               dst_rgb   == that.dst_rgb   &&
               src_alpha == that.src_alpha &&
               dst_alpha == that.dst_alpha;
    }
    void apply() const {
        glBlendFuncSeparate(
            detail::blend_function2gl(src_rgb),
            detail::blend_function2gl(dst_rgb),
            detail::blend_function2gl(src_alpha),
            detail::blend_function2gl(dst_alpha)
        );
    }
};

DEF_STATE_GROUP(depth_state,
    (boolean,    enabled, detail::glSwitch(GL_DEPTH, detail::boolean2gl(enabled))),
    (boolean,    write,   glDepthFunc(detail::boolean2gl(write))),
    (depth_func, func,    glDepthFunc(detail::depth_func2gl(func))),
    (float,      clear,   glClearDepth(clear))
);

DEF_STATE_GROUP(blending_state,
    (boolean,                 enabled,  detail::glSwitch(GL_BLEND, detail::boolean2gl(enabled))),
    (glm::vec4,               color,    glBlendColor(color.r, color.g, color.b, color.a) ),
    (blend_equation_separate, equation, equation.apply()),
    (blend_function_separate, function, function.apply())
);

}