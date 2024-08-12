#ifndef FDUP_H
#define FDUP_H

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

static FILE *fdup(int fd, char const *mode)
{

  int nfd = dup(fd);
  if (nfd < 0) return NULL;
  FILE *fp = fdopen(nfd, mode);
  if (!fp)
  {
    close(nfd);
    return NULL;
  }
  return fp;
}

#endif
