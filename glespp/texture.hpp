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
    unsigned char l;
};

struct alpha {
    unsigned char a;
};

struct luminance_alpha {
    unsigned char l;
    unsigned char a;
};

struct rgb888 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct rgba8888 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
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

#define ADD_PIXEL_TRAIT(pixel_type, gl_format, gl_type, exp_size) \
    template<>                                                    \
    struct pixel_traits<pixel_type> {                             \
        static const GLenum format = gl_format;                   \
        static const GLenum type   = gl_type;                     \
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

inline GLenum textue_dims2gl_enum(textue_dims d) {
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

class texture_base {
public:    
    texture_base() 
        : _id(0)
        , _type(GL_TEXTURE_2D)
    {}

    texture_base(const texture_base&) = delete;
    texture_base(texture_base&& t) 
        : _id(t._id)
        , _type(t._type)
    { t._id = 0; }
    
    GLuint get_id()     const { return _id;   }
    GLenum get_target() const { return _type; }
protected:
    GLuint  _id;
    GLenum  _type;
};

}

class texture_ref {
public:
    texture_ref()
        : id(0)
        , target(GL_TEXTURE_2D)
    {}
    texture_ref(const detail::texture_base& t)
        : id(t.get_id())
        , target(t.get_target())
    {}

    GLuint id;
    GLenum target;
};


template<typename pixel_t>
class texture : public detail::texture_base {
public:
    typedef pixel_t pixel_type;

    texture(uint32_t w, uint32_t h, mips m = mips::one, textue_dims dims = textue_dims::t2d) 
        : _teximage_targets(detail::tex_targets_2d)
    {
        static bool s_unpack_alignment_set = false;
        if (!s_unpack_alignment_set) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            s_unpack_alignment_set = true;
        }

        _type = detail::textue_dims2gl_enum(dims);
        glGenTextures(1, &this->_id);
        glBindTexture(_type, this->_id);

        uint32_t min_ext = std::min(w, h);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (mips::all != m) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            min_ext = 1;
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

    texture(texture&& t)
        : detail::texture_base(std::move(t))
    {}

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

private:
    const GLenum* _teximage_targets;
};

}