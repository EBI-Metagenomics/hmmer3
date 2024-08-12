#ifndef H3RESULT_H3RESULT_STRUCT_H
#define H3RESULT_H3RESULT_STRUCT_H

#include "stats.h"
#include "tophits.h"

struct h3r
{
  int errnum;
  char const *errstr;
  struct stats stats;
  struct tophits tophits;
};

#endif
