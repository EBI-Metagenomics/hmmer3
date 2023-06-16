#ifndef H3RESULT_HIT_H
#define H3RESULT_HIT_H

#include "static_assert.h"
#include <stdint.h>

struct lip_file;
struct domain;

struct hit
{
  char *name;
  char *acc;
  char *desc;
  double sortkey;

  float score;
  float pre_score;
  float sum_score;

  double lnP;
  double pre_lnP;
  double sum_lnP;

  float nexpected;
  unsigned nregions;
  unsigned nclustered;
  unsigned noverlaps;
  unsigned nenvelopes;

  unsigned flags;
  unsigned nreported;
  unsigned nincluded;
  unsigned best_domain;

  unsigned ndomains;
  struct domain *domains;
};

h3result_static_assert(sizeof(unsigned) >= 4);

int h3result_hit_init(struct hit *);
int h3result_hit_setup(struct hit *, unsigned ndomains);
void h3result_hit_cleanup(struct hit *);
int h3result_hit_pack(struct hit const *, struct lip_file *);
int h3result_hit_unpack(struct hit *, struct lip_file *);

#endif
