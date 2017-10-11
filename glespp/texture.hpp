#pragma once

#include <glad/glad.h>

namespace glespp {

enum class textue_dims {
    t2d,
    cubemap
};

enum class mips {
    one,
    all,
    gen,
};

enum class cubemap_face {
    positive_x,
    negative_x,
    positive_y,
    negative_y,
    positive_z,
    negative_z,
};

namespace pixel_format {

struct luminance {
    unsigned char l : 8;
};

struct alpha {
    unsigned char a : 8;
};

struct luminance_alpha {
    unsigned char l : 8;
    unsigned char a : 8;
};

struct rgb888 {
    unsigned char r : 8;
    unsigned char g : 8;
    unsigned char b : 8;
};

struct rgba8888 {
    unsigned char r : 8;
    unsigned char g : 8;
    unsigned char b : 8;
    unsigned char a : 8;
};

struct rgba4444 {
    unsigned char r : 4;
    unsigned char g : 4;
    unsigned char b : 4;
    unsigned char a : 4;
};

struct rgba5551 {
    unsigned short r : 5;
    unsigned short g : 5;
    unsigned short b : 5;
    unsigned short a : 1;
};

struct rgb565 {
    unsigned short r : 5;
    unsigned short g : 6;
    unsigned short b : 5;
};

template<typename pixel_t>
struct pixel_traits;

#define ADD_PIXEL_TRAIT(pixel_type, gl_type, gl_format, exp_size) \
    template<>                                                    \
    struct pixel_traits<pixel_type> {                             \
        static const GLenum type   = gl_type;                     \
        static const GLenum format = gl_format;                   \
    };                                                            \
    static_assert(sizeof(pixel_type) == exp_size, "Unexpected size")

ADD_PIXEL_TRAIT(luminance,       GL_LUMINANCE,       GL_UNSIGNED_BYTE,          1);
ADD_PIXEL_TRAIT(alpha,           GL_ALPHA,           GL_UNSIGNED_BYTE,          1);
ADD_PIXEL_TRAIT(luminance_alpha, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,          2);
ADD_PIXEL_TRAIT(rgb888,          GL_RGB,             GL_UNSIGNED_BYTE,          3);
ADD_PIXEL_TRAIT(rgba8888,        GL_RGBA,            GL_UNSIGNED_BYTE,          4);
ADD_PIXEL_TRAIT(rgba4444,        GL_RGBA,            GL_UNSIGNED_SHORT_4_4_4_4, 2);
ADD_PIXEL_TRAIT(rgba5551,        GL_RGBA,            GL_UNSIGNED_SHORT_5_5_5_1, 2);
ADD_PIXEL_TRAIT(rgb565,          GL_RGB,             GL_UNSIGNED_SHORT_5_6_5,   2);
#undef ADD_PIXEL_TRAIT

}

namespace detail {

GLenum textue_dims2gl_enum(textue_dims d) {
    switch (d) {
    case textue_dims::t2d:     return GL_TEXTURE_2D;
    case textue_dims::cubemap: return GL_TEXTURE_CUBE_MAP;
    default:                   return GL_TEXTURE_2D;
    }
}

static const GLenum tex_targets_2d[] = {
    GL_TEXTURE_2D,
    GL_NONE
};
static const GLenum tex_targets_cubemap[] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    GL_NONE,
};
}

template<typename pixel_t>
class texture {
public:
    typedef pixel_t pixel_type;

    texture(uint32_t w, uint32_t h, mips m = mips::one, textue_dims dims = textue_dims::t2d) 
        : _id(0)
        , _type(detail::textue_dims2gl_enum(dims))
        , _teximage_targets(detail::tex_targets_2d)
    {
        glGenTextures(1, &_id);
        glBindTexture(_type, _id);

        uint32_t min_ext = std::min(w, h);
        if (mips::all != m) {
            min_ext = 1;
        }

        if (dims == textue_dims::cubemap) {
            _teximage_targets = detail::tex_targets_cubemap;
        }

        GLenum format = pixel_format::pixel_traits<pixel_type>::format;
        GLenum type   = pixel_format::pixel_traits<pixel_type>::type;

        for (; min_ext; w >>= 1, h >>= 1, min_ext >>= 1) {
            for(const GLenum* target = _teximage_targets; GL_NONE != *target; ++target) {
                glTexImage2D(*target, 0, format, w, h, 0, format, type, nullptr);
            }
        }

        if (mips::gen == m) {
            glGenerateMipmap(_type);
        }
    }

    ~texture() {
        glDeleteTextures(1, &_id);
    }

    void update(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t w, uint32_t h, pixel_type* data) {
        assert(_type == GL_TEXTURE_2D);
        glBindTexture(_type, _id);

        GLenum format = pixel_format::pixel_traits<pixel_type>::format;
        GLenum type   = pixel_format::pixel_traits<pixel_type>::type;

        glTexSubImage2D(_type, level, xoffset, yoffset, w, h, format, type, data);
    }
    
    void update(cubemap_face face, uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t w, uint32_t h, pixel_type* data) {
        assert(_type == GL_TEXTURE_CUBE_MAP);
        glBindTexture(_type, _id);

        GLenum format = pixel_format::pixel_traits<pixel_type>::format;
        GLenum type   = pixel_format::pixel_traits<pixel_type>::type;

        GLenum gl_face = detail::tex_targets_cubemap[static_cast<size_t>(face)];
        glTexSubImage2D(gl_face, level, xoffset, yoffset, w, h, format, type, data);
    }

    GLuint get_id() { return _id; }

private:
    GLuint  _id;
    GLenum  _type;
    const GLenum* _teximage_targets;
};

}