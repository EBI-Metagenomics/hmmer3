#ifndef PARSE_H
#define PARSE_H

#include "little_endian.h"
#include <stdint.h>

static inline uint32_t parse_uint32(unsigned char const x[4])
{
  if (little_endian())
  {
    return (uint32_t)x[0] << 24|
           (uint32_t)x[1] << 16|
           (uint32_t)x[2] <<  8|
           (uint32_t)x[3] <<  0;
  }
  else
  {
    return (uint32_t)x[0] <<  0|
           (uint32_t)x[1] <<  8|
           (uint32_t)x[2] << 16|
           (uint32_t)x[3] << 24;
  }
}

static inline uint64_t parse_uint64(unsigned char const x[8])
{
  if (little_endian())
  {
    return (uint64_t)x[0] << 56|
           (uint64_t)x[1] << 48|
           (uint64_t)x[2] << 40|
           (uint64_t)x[3] << 32|
           (uint64_t)x[4] << 24|
           (uint64_t)x[5] << 16|
           (uint64_t)x[6] <<  8|
           (uint64_t)x[7] <<  0;
  }
  else
  {
    return (uint64_t)x[0] <<  0|
           (uint64_t)x[1] <<  8|
           (uint64_t)x[2] << 16|
           (uint64_t)x[3] << 24|
           (uint64_t)x[4] << 32|
           (uint64_t)x[5] << 40|
           (uint64_t)x[6] << 48|
           (uint64_t)x[7] << 56;
  }
}

static inline float parse_float(unsigned char const x[4])
{
  return (union {uint32_t u; float f;}){.u = parse_uint32(x)}.f;
}

static inline double parse_double(unsigned char const x[8])
{
  return (union {uint64_t u; double f;}){.u = parse_uint64(x)}.f;
}

static inline uint8_t parse_flag(unsigned char const x[1]) { return x[0]; }

#endif
