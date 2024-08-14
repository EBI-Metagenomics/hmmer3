#include "h3c_result.h"
#include "array_size.h"
#include "h3r_result.h"
#include <stdlib.h>
#include <string.h>

struct h3c_result *h3c_result_new(void)
{
  struct h3c_result *x = malloc(sizeof(*x));
  if (!x) return NULL;

  if (!(x->content = h3r_new()))
  {
    free(x);
    return NULL;
  }
  x->errnum = 0;
  memset(x->errstr, '\0', array_size(x->errstr));
  return x;
}

void h3c_result_del(struct h3c_result const *x)
{
  if (x)
  {
    h3r_del(x->content);
    free((void *)x);
  }
}
