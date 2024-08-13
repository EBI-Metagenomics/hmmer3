#include "zc.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static uint32_t zc_byteswap32(uint32_t x)
{
  return (uint32_t)(x >> 24 | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) |
                    x << 24);
}

static uint64_t zc_byteswap64(uint64_t x)
{
  return (uint64_t)((zc_byteswap32(x) + 0ULL) << 32 | zc_byteswap32(x >> 32));
}

enum
{
  ZC_ENDIAN_LITTLE = 1234,
  ZC_ENDIAN_BIG = 4321,
};

#ifdef __STDC_ENDIAN_NATIVE__
#if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
#define ZC_ENDIAN_NATIVE ZC_ENDIAN_LITTLE
#elif __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_BIG__
#define ZC_ENDIAN_NATIVE ZC_ENDIAN_BIG
#else
#error "Unknown native byte order"
#endif
#endif

#ifdef ZC_ENDIAN_NATIVE

#if ZC_ENDIAN_NATIVE == ZC_ENDIAN_LITTLE

static uint32_t zc_htonl(uint32_t x) { return zc_byteswap32(x); }
static uint64_t zc_htonll(uint64_t x) { return zc_byteswap64(x); }

uint32_t zc_ntohl(uint32_t x) { return zc_htonl(x); }
uint64_t zc_ntohll(uint64_t x) { return zc_htonll(x); }

#else
static uint32_t zc_htonl(uint32_t x) { return x; }
static uint64_t zc_htonll(uint64_t x) { return x; }

uint32_t zc_ntohl(uint32_t x) { return x; }
uint64_t zc_ntohll(uint64_t x) { return x; }
#endif

#else

static int zc_endianness(void)
{
  union
  {
    uint32_t value;
    uint8_t data[sizeof(uint32_t)];
  } number;

  number.data[0] = 0x00;
  number.data[1] = 0x01;
  number.data[2] = 0x02;
  number.data[3] = 0x03;

  switch (number.value)
  {
  case (uint32_t)(0x00010203):
    return ZC_ENDIAN_BIG;
  case (uint32_t)(0x03020100):
    return ZC_ENDIAN_LITTLE;
  default:
    exit(1);
    return -1;
  }
}

static uint32_t zc_htonl(uint32_t x)
{
  if (zc_endianness() == ZC_ENDIAN_BIG) return x;
  return zc_byteswap32(x);
}

static uint64_t zc_htonll(uint64_t x)
{
  if (zc_endianness() == ZC_ENDIAN_BIG) return x;
  return zc_byteswap64(x);
}

uint32_t zc_ntohl(uint32_t x) { return zc_htonl(x); }
uint64_t zc_ntohll(uint64_t x) { return zc_htonll(x); }

#endif

void *zc_reallocf(void *ptr, size_t size)
{
  assert(size > 0);
  void *new_ptr = realloc(ptr, size);

  if (!new_ptr) free(ptr);
  return new_ptr;
}
