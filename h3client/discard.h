#ifndef DISCARD_H
#define DISCARD_H

#include "full_recv.h"

static inline int discard(int fd, size_t size)
{
  unsigned char buffer[128] = {0};
  return full_recv(fd, size, buffer);
}

#endif
