#ifndef BSD_H
#define BSD_H

#include <stddef.h>
#include <stdlib.h>

static inline void *bsd_reallocf(void *ptr, size_t size)
{
  if (size <= 0) return NULL;
  void *new_ptr = realloc(ptr, size);

  if (!new_ptr) free(ptr);
  return new_ptr;
}

#endif
