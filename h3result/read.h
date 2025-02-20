#ifndef READ_H
#define READ_H

#include "bsd.h"
#include "lio.h"
#include <stdint.h>

static inline int read_map(struct lio_reader *x, uint32_t *size)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_map(buf, size))) return 1;
  return 0;
}

static inline int read_array(struct lio_reader *x, uint32_t *size)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_array(buf, size))) return 1;
  return 0;
}

static inline int read_string(struct lio_reader *x, uint32_t *size)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_string(buf, size))) return 1;
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

static inline int read_cstring2(struct lio_reader *f, char **str)
{
  unsigned size = 0;
  if (read_string(f, &size)) return 0;
  if (!(*str = bsd_reallocf(*str, size + 1))) return 0;
  if (lio_readb(f, size, (unsigned char *)*str)) return 0;
  (*str)[size] = '\0';
  return 0;
}

static inline int read_f32(struct lio_reader *x, float *data)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_float(buf, data))) return 1;
  return 0;
}

static inline int read_f64(struct lio_reader *x, double *data)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_float(buf, data))) return 1;
  return 0;
}

static inline int read_unsigned(struct lio_reader *x, unsigned *data)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_int(buf, data))) return 1;
  return 0;
}

static inline int read_ulong(struct lio_reader *x, unsigned long *data)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_int(buf, data))) return 1;
  return 0;
}

static inline int read_bool(struct lio_reader *x, bool *data)
{
  unsigned char *buf = NULL;
  if (lio_read(x, &buf)) return 1;
  if (lio_free(x, lip_unpack_bool(buf, data))) return 1;
  return 0;
}

#define read_int(stream, data)                                                 \
  _Generic((data), unsigned *: read_unsigned, unsigned long *: read_ulong)(    \
      stream, data)

#define read_float(stream, data)                                               \
  _Generic((data), float *: read_f32, double *: read_f64)(stream, data)

#endif
