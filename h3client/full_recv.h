#ifndef FULL_RECV_H
#define FULL_RECV_H

#include <sys/socket.h>

static inline int full_recv(int fd, size_t size, unsigned char *buffer)
{
  while (size > 0)
  {
    ssize_t n = recv(fd, buffer, size, 0);
    if (n == -1 || n == 0) return 1;
    buffer += n;
    size -= (size_t)n;
  }
  return 0;
}

#endif
