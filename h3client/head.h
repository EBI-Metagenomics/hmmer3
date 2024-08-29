#ifndef HEAD_H
#define HEAD_H

#include "parse.h"
#include <stddef.h>
#include <stdint.h>

struct head
{
  uint32_t status;
  uint64_t msglen;
};

#define HEAD_SIZE (sizeof(uint32_t) + sizeof(uint64_t))

static void head_parse(struct head *x, unsigned char const buffer[HEAD_SIZE])
{
  x->status = parse_uint32(buffer);
  x->msglen = parse_uint64(buffer + 4);
}

#endif
