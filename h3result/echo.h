#ifndef ECHO_H
#define ECHO_H

#include "defer_return.h"
#include <stdarg.h>
#include <stdio.h>

static inline int __attribute__((format(printf, 2, 3)))
echon(FILE *fp, char const *fmt, ...)

{
  int rc = 0;
  va_list params = {0};
  va_start(params, fmt);
  if (vfprintf(fp, fmt, params) < 0) defer_return(1);
  if (fputc('\n', fp) < 0) defer_return(1);
  va_end(params);
  return 0;

defer:
  va_end(params);
  return rc;
}

static inline int __attribute__((format(printf, 2, 3)))
echo(FILE *fp, char const *fmt, ...)

{
  int rc = 0;
  va_list params = {0};
  va_start(params, fmt);
  if (vfprintf(fp, fmt, params) < 0) defer_return(1);
  va_end(params);
  return 0;

defer:
  va_end(params);
  return rc;
}

#endif
