#ifndef WRITE_H
#define WRITE_H

#include "lio.h"
#include <string.h>

static inline int write_cstring(struct lio_writer *x, char const *string)
{
  uint32_t length = (uint32_t)strlen(string);
  if (lio_write(x, lip_pack_string(lio_alloc(x), length))) return 1;
  if (lio_writeb(x, length, string)) return 1;
  return 0;
}

static inline int write_f32(struct lio_writer *x, float data)
{
  if (lio_write(x, lip_pack_float(lio_alloc(x), data))) return 1;
  return 0;
}

static inline int write_f64(struct lio_writer *x, double data)
{
  if (lio_write(x, lip_pack_float(lio_alloc(x), data))) return 1;
  return 0;
}

static inline int write_int(struct lio_writer *x, int data)
{
  if (lio_write(x, lip_pack_int(lio_alloc(x), data))) return 1;
  return 0;
}

static inline int write_bool(struct lio_writer *x, bool data)
{
  if (lio_write(x, lip_pack_bool(lio_alloc(x), data))) return 1;
  return 0;
}

static inline int write_array(struct lio_writer *x, uint32_t size)
{
  if (lio_write(x, lip_pack_array(lio_alloc(x), size))) return 1;
  return 0;
}

static inline int write_map(struct lio_writer *x, uint32_t size)
{
  if (lio_write(x, lip_pack_map(lio_alloc(x), size))) return 1;
  return 0;
}

#define write_float(stream, data)                                              \
  _Generic((data), float: write_f32, double: write_f64)(stream, data)

#endif
