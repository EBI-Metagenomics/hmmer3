#include "echo.h"
#include <stdarg.h>

void h3client_echo(FILE *stream, char const *fmt, ...)
{
  va_list params = {0};
  va_start(params, fmt);
  vfprintf(stream, fmt, params);
  fputc('\n', stream);
  va_end(params);
}
