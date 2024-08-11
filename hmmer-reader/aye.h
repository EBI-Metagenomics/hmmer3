#ifndef AYE_H
#define AYE_H

#include <stdio.h>

#define AYE_RED "\033[0;31m"
#define AYE_CLEAR "\033[0m"
#define AYE_GREEN "\033[0;32m"

#define AYE_XSTRING(s) AYE_STRING(s)
#define AYE_STRING(s) #s

static int aye_success_count = 0;
static int aye_failure_count = 0;

static void aye_begin_impl(char const *file)
{
  aye_success_count = 0;
  aye_failure_count = 0;
  fprintf(stderr, "%s ", file);
}

#define aye_begin() aye_begin_impl(__FILE__)

static int aye_end(void)
{
  if (!aye_failure_count) fprintf(stderr, AYE_GREEN " PASSED" AYE_CLEAR "\n");
  return aye_failure_count > 0 ? 1 : 0;
}

#define aye(expr)                                                              \
  do                                                                           \
  {                                                                            \
    if ((expr))                                                                \
    {                                                                          \
      aye_success_count++;                                                     \
      if (!aye_failure_count) fprintf(stderr, AYE_GREEN "." AYE_CLEAR);        \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      aye_failure_count++;                                                     \
      if (aye_failure_count == 1) fprintf(stderr, "\n");                       \
      fprintf(stderr, AYE_RED "E %s:%-6d ", __FILE__, __LINE__);               \
      fputs(AYE_XSTRING(expr) AYE_CLEAR "\n", stderr);                         \
    }                                                                          \
  } while (0)

#endif
