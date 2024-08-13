#ifndef STRUCT_H
#define STRUCT_H

#include "h3r_stats.h"
#include "h3r_tophits.h"

struct h3r
{
  struct stats stats;
  struct tophits tophits;
};

#endif
