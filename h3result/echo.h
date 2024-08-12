#ifndef ECHO_H
#define ECHO_H

#include <stdarg.h>
#include <stdio.h>

static inline void __attribute__((format(printf, 2, 3)))
h3result_echo(FILE *fp, char const *fmt, ...)

{
  va_list params = {0};
  va_start(params, fmt);
  vfprintf(fp, fmt, params);
  fputc('\n', fp);
  va_end(params);
}

#endif
