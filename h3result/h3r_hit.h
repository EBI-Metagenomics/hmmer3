#ifndef H3R_HIT_H
#define H3R_HIT_H

#include <stdint.h>

struct h3r_domain;
struct lio_writer;
struct lio_reader;

struct h3r_hit
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
  struct h3r_domain *domains;
};

int  h3r_hit_init(struct h3r_hit *);
int  h3r_hit_setup(struct h3r_hit *, unsigned ndomains);
void h3r_hit_cleanup(struct h3r_hit *);
int  h3r_hit_pack(struct h3r_hit const *, struct lio_writer *);
int  h3r_hit_unpack(struct h3r_hit *, struct lio_reader *);

#endif
