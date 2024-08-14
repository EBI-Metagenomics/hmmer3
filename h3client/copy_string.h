#ifndef COPY_STRING
#define COPY_STRING

#include <limits.h>
#include <stddef.h>
#include <string.h>

static int copy_string(size_t size, char dst[size], char const src[static 1])
{
  if (size == 0) return 1;
  size_t count = strlen(src);
  if (count >= size) return 1;
  memcpy(dst, src, count);
  dst[count] = '\0';
  return 0;
}

#endif
