#ifndef FDUP_H
#define FDUP_H

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

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
