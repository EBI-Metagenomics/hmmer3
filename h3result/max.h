#ifndef H3RESULT_MAX_H
#define H3RESULT_MAX_H

#include <stddef.h>

static inline unsigned max_u(unsigned a, unsigned b) { return a > b ? a : b; }

static inline unsigned long max_ul(unsigned long a, unsigned long b)
{
  return a > b ? a : b;
}

static inline unsigned long long max_ull(unsigned long long a,
                                         unsigned long long b)
{
  return a > b ? a : b;
}

#define max(a, b)                                                              \
  _Generic((a),                                                                \
      unsigned: max_u,                                                         \
      unsigned long: max_ul,                                                   \
      unsigned long long: max_ull)(a, b)

#endif
