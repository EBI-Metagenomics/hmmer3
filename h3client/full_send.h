#ifndef FULL_SEND_H
#define FULL_SEND_H

#include <sys/socket.h>

static inline int full_send(int fd, size_t size, unsigned char const *buffer)
{
  while (size > 0)
  {
    ssize_t n = send(fd, buffer, size, 0);
    if (n == -1) return 1;
    buffer += n;
    size -= (size_t)n;
  }
  return 0;
}

#endif
