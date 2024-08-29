#ifndef LITTLE_ENDIAN_H
#define LITTLE_ENDIAN_H

#include <stdint.h>

static inline int little_endian(void)
{
  const union
  {
    uint32_t u;
    uint8_t c[4];
  } one = {.u = 1};
  return one.c[0];
}

#endif
