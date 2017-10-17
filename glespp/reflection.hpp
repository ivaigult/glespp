#pragma once

#include <cstddef>

#define __PP_CAT(x0, x1) x0 ## x1
#define __PP_EVAL(...) __VA_ARGS__

#define __PP_EMPTY() 
#define __PP_DEFER_0(fn) fn __PP_EMPTY()
#define __PP_DEFER_1(fn) fn __PP_EMPTY __PP_EMPTY () ()
#define __PP_DEFER_2(fn) fn __PP_EMPTY __PP_EMPTY __PP_EMPTY () () ()

#define PP_ARG_LEN(...) __PP_EVAL(__PP_ARG_LEN(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define __PP_ARG_LEN(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, num_args, ...) num_args

#define __PP_FOR_EACH_0(fn,  cookie,    ...)
#define __PP_FOR_EACH_1(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_0( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_2(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_1( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_3(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_2( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_4(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_3( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_5(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_4( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_6(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_5( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_7(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_6( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_8(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_7( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_9(fn,  cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_8( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_10(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_9( fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_11(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_10(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_12(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_11(fn, cookie, __VA_ARGS__))

#define PP_FOR_EACH(fn, cookie, ...) __PP_FOR_EACH(PP_ARG_LEN(__VA_ARGS__), fn, cookie, ##__VA_ARGS__)
#define __PP_FOR_EACH(n, fn, cookie, ...) __PP_EVAL(__PP_CAT(__PP_FOR_EACH_, n) (fn, cookie, ##__VA_ARGS__))

#define PP_IMPL_VISIT(cookie, type_field_pair) __PP_EVAL(__PP_IMPL_VISIT type_field_pair)
#define __PP_IMPL_VISIT(type, field_name) visitor(#field_name, field_name);

#define PP_IMPL_MEMBER(cookie, type_field_pair) __PP_EVAL(__PP_IMPL_MEMBER type_field_pair)
#define __PP_IMPL_MEMBER(type, field_name) type field_name;

#define DEF_REFLECTABLE(name, ...)                              \
    struct name {                                               \
        static const char* get_name() { return #name; }         \
        template<typename visitor_t>                            \
        void foreach_member(visitor_t& visitor) const {         \
            PP_FOR_EACH(PP_IMPL_VISIT, _, ##__VA_ARGS__)        \
        }                                                       \
        PP_FOR_EACH(PP_IMPL_MEMBER, _, ##__VA_ARGS__)           \
    }
