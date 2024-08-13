#ifndef ZC_H
#define ZC_H

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#if SHRT_MAX == INT8_MAX
#define ZC_BYTES_PER_SHORT 1
#define ZC_SHORT_WIDTH 8
#elif SHRT_MAX == INT16_MAX
#define ZC_BYTES_PER_SHORT 2
#define ZC_SHORT_WIDTH 16
#elif SHRT_MAX == INT32_MAX
#define ZC_BYTES_PER_SHORT 4
#define ZC_SHORT_WIDTH 32
#elif SHRT_MAX == INT64_MAX
#define ZC_BYTES_PER_SHORT 8
#define ZC_SHORT_WIDTH 64
#else
#error "Cannot determine size of short"
#endif

#if INT_MAX == INT8_MAX
#define ZC_BYTES_PER_INT 1
#define ZC_INT_WIDTH 8
#elif INT_MAX == INT16_MAX
#define ZC_BYTES_PER_INT 2
#define ZC_INT_WIDTH 16
#elif INT_MAX == INT32_MAX
#define ZC_BYTES_PER_INT 4
#define ZC_INT_WIDTH 32
#elif INT_MAX == INT64_MAX
#define ZC_BYTES_PER_INT 8
#define ZC_INT_WIDTH 64
#else
#error "Cannot determine size of int"
#endif

#if LONG_MAX == INT8_MAX
#define ZC_BYTES_PER_LONG 1
#define ZC_LONG_WIDTH 8
#elif LONG_MAX == INT16_MAX
#define ZC_BYTES_PER_LONG 2
#define ZC_LONG_WIDTH 16
#elif LONG_MAX == INT32_MAX
#define ZC_BYTES_PER_LONG 4
#define ZC_LONG_WIDTH 32
#elif LONG_MAX == INT64_MAX
#define ZC_BYTES_PER_LONG 8
#define ZC_LONG_WIDTH 64
#else
#error "Cannot determine size of long"
#endif

enum
{
  ZC_WINDOWS,
  ZC_UNIX,
  ZC_MACOS,
};

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#define ZC_OS ZS_WINDOWS
#elif defined(__unix__) || defined(__unix)
#define ZC_OS ZS_UNIX
#elif defined(__APPLE__)
#define ZC_OS ZC_MACOS
#endif

#ifdef MS_WINDOWS
#define ZC_PATH_SEP '\\'
#endif

#ifndef ZC_PATH_SEP
#define ZC_PATH_SEP '/'
#endif

#define zc_array_size(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * zc_container_of - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define zc_container_of(ptr, type, member)                                     \
  ({                                                                           \
    char *__mptr = (char *)(ptr);                                              \
    ((type *)(__mptr - offsetof(type, member)));                               \
  })

/**
 * zc_container_of_safe - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * Return NULL if ptr is NULL.
 */
#define zc_container_of_safe(ptr, type, member)                                \
  ({                                                                           \
    char *__mptr = (char *)(ptr);                                              \
    __mptr == NULL ? (type *)__mptr                                            \
                   : ((type *)(__mptr - offsetof(type, member)));              \
  })

uint16_t zc_ntohs(uint16_t);
uint32_t zc_ntohl(uint32_t);
uint64_t zc_ntohll(uint64_t);

void *zc_reallocf(void *ptr, size_t size);

#endif
