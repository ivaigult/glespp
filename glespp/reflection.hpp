#pragma once

#include "detail/cpp_foreach.hpp"


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
