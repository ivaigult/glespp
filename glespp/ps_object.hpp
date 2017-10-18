#pragma once

#include "reflection.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <limits>

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

#define PP_IMPL_STATE_MEMBER(cookie, type_name_def_apply) __PP_EVAL(__PP_IMPL_STATE_MEMBER type_name_def_apply)
#define __PP_IMPL_STATE_MEMBER(type, name, def, apply) type name;

#define PP_IMPL_STATE_INIT(cookie, type_name_def_apply) __PP_EVAL(__PP_IMPL_STATE_INIT type_name_def_apply)
#define __PP_IMPL_STATE_INIT(type, name, def, apply) name = def;


#define PP_IMPL_STATE_APPLY(cookie, type_name_def_apply) __PP_EVAL(__PP_IMPL_STATE_APPLY type_name_def_apply)
#define __PP_IMPL_STATE_APPLY(type, name, def, apply) apply;

#define DEF_STATE_GROUP(name, ...)                              \
    struct name {                                               \
        name() {                                                \
            PP_FOR_EACH(PP_IMPL_STATE_INIT, _, ##__VA_ARGS__);  \
        }                                                       \
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
    (on,  GL_TRUE ),
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

DEF_STATE_ENUM(face_orientation,
    (cw,  GL_CW ),
    (ccw, GL_CCW)
);

DEF_STATE_ENUM(face,
    (front,          GL_FRONT         ),
    (back,           GL_BACK          ),
    (front_and_back, GL_FRONT_AND_BACK)
)

DEF_STATE_ENUM(blend_equation,
    (add,              GL_FUNC_ADD             ), 
    (subtract,         GL_FUNC_SUBTRACT        ),
    (reverse_subtract, GL_FUNC_REVERSE_SUBTRACT)
);

DEF_STATE_ENUM(blend_function,
    (zero,                     GL_ZERO                    ),
    (one,                      GL_ONE                     ),
    (src_color,                GL_SRC_COLOR               ),
    (one_minus_src_color,      GL_ONE_MINUS_SRC_COLOR     ),
    (dst_color,                GL_DST_COLOR               ),
    (one_minus_dst_color,      GL_ONE_MINUS_DST_COLOR     ),
    (src_alpha,                GL_SRC_ALPHA               ),
    (one_minus_src_alpha,      GL_ONE_MINUS_SRC_ALPHA     ),
    (dst_alpha,                GL_DST_ALPHA               ),
    (one_minus_dst_alpha,      GL_ONE_MINUS_DST_ALPHA     ),
    (constant_color,           GL_CONSTANT_COLOR          ),
    (one_minus_constant_color, GL_ONE_MINUS_CONSTANT_COLOR),
    (constant_alpha,           GL_CONSTANT_ALPHA          ),
    (one_minus_constant_alpha, GL_ONE_MINUS_CONSTANT_ALPHA),
    (src_alpha_saturate,       GL_SRC_ALPHA_SATURATE      )
);

DEF_STATE_ENUM(stencil_func,
    (never,     GL_NEVER   ),
    (less,      GL_LESS    ),
    (lequal,    GL_LEQUAL  ),
    (greater,   GL_GREATER ),
    (gequal,    GL_GEQUAL  ),
    (equal,     GL_EQUAL   ),
    (not_equal, GL_NOTEQUAL),
    (always,    GL_ALWAYS  )
);

DEF_STATE_ENUM(stencil_op,
    (keep,      GL_KEEP     ),
    (zero,      GL_ZERO     ),
    (replace,   GL_REPLACE  ),
    (incr,      GL_INCR     ),
    (incr_wrap, GL_INCR_WRAP),
    (decr,      GL_DECR     ),
    (decr_wrap, GL_DECR_WRAP),
    (invert,    GL_INVERT   )
);

struct stencil_param
{
    stencil_func func;
    int32_t      ref;
    uint32_t     mask;
    stencil_op   sfail;
    stencil_op   dpfail;
    stencil_op   dppass;
    bool operator=(const stencil_param& that) const {
        return func   == that.func    &&
               ref    == that.ref     &&
               mask   == that.mask    &&
               sfail  == that.sfail   &&
               dpfail == that.dpfail &&
               dppass == that.dppass;
    }
    void apply(GLenum face) const {
        glStencilFuncSeparate(face, detail::stencil_func2gl(func), ref, mask);
        glStencilOpSeparate(face, 
            detail::stencil_op2gl(sfail),
            detail::stencil_op2gl(dpfail),
            detail::stencil_op2gl(dppass)
        );
    }
};

DEF_STATE_GROUP(depth_state,
    (boolean,    enabled,   boolean::off,     detail::glSwitch(GL_DEPTH_TEST, detail::boolean2gl(enabled))),
    (boolean,    write,     boolean::on,      glDepthFunc(detail::boolean2gl(write))),
    (depth_func, func,      depth_func::less, glDepthFunc(detail::depth_func2gl(func)))
);

DEF_STATE_GROUP(stencil_state,
    (boolean, enabled, boolean::off, detail::glSwitch(GL_STENCIL_TEST, detail::boolean2gl(enabled))),
    (stencil_param, front, 
        stencil_param({
            stencil_func::always, 0, std::numeric_limits<uint32_t>::max(),
            stencil_op::keep, stencil_op::keep, stencil_op::keep
        }), front.apply(GL_FRONT)),
    (stencil_param, back, 
        stencil_param({
            stencil_func::always, 0, std::numeric_limits<uint32_t>::max(),
            stencil_op::keep, stencil_op::keep, stencil_op::keep
        }), front.apply(GL_BACK))
);

DEF_STATE_GROUP(rasterization_state,
    (face_orientation, orientation, face_orientation::ccw, glFrontFace(detail::face_orientation2gl(orientation))),
    (boolean,          enabled,     boolean::off,          detail::glSwitch(GL_CULL_FACE, detail::boolean2gl(enabled))),
    (face,             cull_face,   face::back,            glCullFace(detail::face2gl(cull_face)))
);

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

DEF_STATE_GROUP(blending_state,
    (boolean,                 enabled,  boolean::off,                  detail::glSwitch(GL_BLEND, detail::boolean2gl(enabled))),
    (glm::vec4,               color,    glm::vec4(0.f, 0.f, 0.f, 0.f), glBlendColor(color.r, color.g, color.b, color.a) ),
    (blend_equation_separate, equation, 
        blend_equation_separate({blend_equation::add, blend_equation::add }), equation.apply()),
    (blend_function_separate, function, 
        blend_function_separate({ blend_function::one, blend_function::one, blend_function::zero, blend_function::zero}), function.apply())
);

} // glespp
