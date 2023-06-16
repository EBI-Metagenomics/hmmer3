#include "reallocf.h"
#include <assert.h>
#include <stdlib.h>

void *h3result_reallocf(void *ptr, size_t size)
{
  assert(size > 0);
  void *new_ptr = realloc(ptr, size);

  if (!new_ptr) free(ptr);
  return new_ptr;
}
