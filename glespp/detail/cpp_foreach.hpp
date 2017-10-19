#pragma once

#define __PP_CAT(x0, x1) x0 ## x1
#define __PP_EVAL(...) __VA_ARGS__

#define __PP_EMPTY() 
#define __PP_DEFER_0(fn) fn __PP_EMPTY()
#define __PP_DEFER_1(fn) fn __PP_EMPTY __PP_EMPTY () ()
#define __PP_DEFER_2(fn) fn __PP_EMPTY __PP_EMPTY __PP_EMPTY () () ()

#define PP_CONS(el, list)  (el, __PP_EVAL(__PP_EVAL list))

#define PP_ARG_LEN(...) __PP_EVAL(__PP_ARG_LEN(__VA_ARGS__, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define __PP_ARG_LEN(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, num_args, ...) num_args

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
#define __PP_FOR_EACH_13(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_12(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_14(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_13(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_15(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_14(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_16(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_15(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_17(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_16(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_18(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_17(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_19(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_18(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_20(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_19(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_21(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_20(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_22(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_21(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_23(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_22(fn, cookie, __VA_ARGS__))
#define __PP_FOR_EACH_24(fn, cookie, x, ...) fn(cookie, x) __PP_EVAL(__PP_FOR_EACH_23(fn, cookie, __VA_ARGS__))

#define PP_FOR_EACH(fn, cookie, ...) __PP_FOR_EACH(PP_ARG_LEN(__VA_ARGS__), fn, cookie, ##__VA_ARGS__)
#define __PP_FOR_EACH(n, fn, cookie, ...) __PP_EVAL(__PP_CAT(__PP_FOR_EACH_, n) (fn, cookie, ##__VA_ARGS__))
