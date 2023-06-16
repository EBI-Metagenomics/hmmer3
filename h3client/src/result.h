#ifndef RESULT_H
#define RESULT_H

#include "stats.h"
#include "tophits.h"

struct h3client_result
{
  int errnum;
  char const *errstr;
  struct stats stats;
  struct tophits tophits;
};

#endif
