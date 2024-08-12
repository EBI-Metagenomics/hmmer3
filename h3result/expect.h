#ifndef EXPECT_H
#define EXPECT_H

#include "read.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct lio_reader;

static inline int expect_key(struct lio_reader *x, char const *key)
{
  char str[16] = {0};
  unsigned size = (unsigned)strlen(key) + 1;
  if (read_cstring(x, size, str)) return 1;
  return strncmp(str, key, sizeof(str)) != 0;
}

static inline int expect_array(struct lio_reader *x, unsigned size)
{
  unsigned sz = 0;
  if (read_array(x, &sz)) return 1;
  return size != sz;
}

static inline int expect_map(struct lio_reader *x, unsigned size)
{
  unsigned sz = 0;
  if (read_map(x, &sz)) return 1;
  return size != sz;
}

#endif
