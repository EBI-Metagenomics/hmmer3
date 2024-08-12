#ifndef READ_H
#define READ_H

#include "lio.h"
#include <stdint.h>

static inline int read_map(struct lio_reader *x, uint32_t *size)
{
  if (lio_free(x, lip_unpack_map(lio_read(x), size))) return 1;
  return 0;
}

static inline int read_array(struct lio_reader *x, uint32_t *size)
{
  if (lio_free(x, lip_unpack_array(lio_read(x), size))) return 1;
  return 0;
}

static inline int read_string(struct lio_reader *x, uint32_t *size)
{
  if (lio_free(x, lip_unpack_string(lio_read(x), size))) return 1;
  return 0;
}

static inline int read_cstring(struct lio_reader *x, uint32_t size,
                               char *string)
{
  uint32_t length = 0;
  if (read_string(x, &length)) return 1;
  if (length >= size) return 1;
  if (lio_readb(x, length, (unsigned char *)string)) return 1;
  string[length] = '\0';
  return 0;
}

static inline int read_f32(struct lio_reader *x, float *data)
{
  if (lio_free(x, lip_unpack_float(lio_read(x), data))) return 1;
  return 0;
}

static inline int read_f64(struct lio_reader *x, double *data)
{
  if (lio_free(x, lip_unpack_float(lio_read(x), data))) return 1;
  return 0;
}

static inline int read_unsigned(struct lio_reader *x, unsigned *data)
{
  if (lio_free(x, lip_unpack_int(lio_read(x), data))) return 1;
  return 0;
}

static inline int read_ulong(struct lio_reader *x, unsigned long *data)
{
  if (lio_free(x, lip_unpack_int(lio_read(x), data))) return 1;
  return 0;
}

static inline int read_bool(struct lio_reader *x, bool *data)
{
  if (lio_free(x, lip_unpack_bool(lio_read(x), data))) return 1;
  return 0;
}

#define read_int(buffer, data)                                                 \
  _Generic((data), unsigned *: read_unsigned, unsigned long *: read_ulong)(    \
      buffer, data)

#define read_float(buffer, data)                                               \
  _Generic((data), float *: read_f32, double *: read_f64)(buffer, data)

#endif
