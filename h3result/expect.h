#ifndef H3RESULT_EXPECT_H
#define H3RESULT_EXPECT_H

#include "read.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct lio_reader;

static inline bool expect_key(struct lio_reader *f, char const *key)
{
  char str[16] = {0};
  unsigned size = (unsigned)strlen(key) + 1;
  if (read_cstring(f, size, str)) return false;
  return strncmp(str, key, sizeof(str)) == 0;
}

static inline bool expect_array(struct lio_reader *f, unsigned size)
{
  unsigned sz = 0;
  if (read_array(f, &sz)) return false;
  return size == sz;
}

static inline bool expect_map(struct lio_reader *f, unsigned size)
{
  unsigned sz = 0;
  if (read_map(f, &sz)) return false;
  return size == sz;
}

#endif
