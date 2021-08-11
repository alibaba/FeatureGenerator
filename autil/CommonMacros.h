#pragma once

#define DELETE_AND_SET_NULL(x) do {             \
        if ((x) != NULL) {                      \
            delete (x);                         \
            (x) = NULL;                         \
        }                                       \
    } while (false)

#define ARRAY_DELETE_AND_SET_NULL(x) do {       \
        if ((x) != NULL) {                      \
            delete[] (x);                       \
            (x) = NULL;                         \
        }                                       \
    } while (false)

#ifndef likely
#define likely(x) __builtin_expect((x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif
//////////////////////////// copy from protobuf/stubs/common.h ///////////////////////
// The COMPILE_ASSERT macro can be used to verify that a compile time
// expression is true. For example, you could use it to verify the
// size of a static array:
//
//   COMPILE_ASSERT(ARRAYSIZE(content_type_names) == CONTENT_NUM_TYPES,
//                  content_type_names_incorrect_size);
//
// or to make sure a struct is smaller than a certain size:
//
//   COMPILE_ASSERT(sizeof(foo) < 128, foo_too_large);
//
// The second argument to the macro is the name of the variable. If
// the expression is false, most compilers will issue a warning/error
// containing the name of the variable.

#if defined(__cplusplus) && (__cplusplus >= 201703L)
#  define AUTIL_NODISCARD [[nodiscard]]
#else
#  define AUTIL_NODISCARD __attribute__((warn_unused_result))
#endif

