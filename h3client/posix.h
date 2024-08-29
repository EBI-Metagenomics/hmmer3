#ifndef POSIX_H
#define POSIX_H

#include <string.h>

static inline size_t posix_strnlen(char const *s, size_t n)
{
  const char *p = memchr(s, 0, n);
  return p ? (size_t)(p - s) : n;
}

#endif
