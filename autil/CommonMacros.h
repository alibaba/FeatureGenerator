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

#if defined(__cplusplus) && (__cplusplus >= 201703L)
#  define AUTIL_NODISCARD [[nodiscard]]
#else
#  define AUTIL_NODISCARD __attribute__((warn_unused_result))
#endif

