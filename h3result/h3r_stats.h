#ifndef H3R_STATS_H
#define H3R_STATS_H

#include "static_assert2.h"
#include "zsetby.h"
#include <stdint.h>

struct h3r_stats;
struct lio_writer;
struct lio_reader;

struct h3r_stats
{
  double Z;
  double domZ;

  enum zsetby Z_setby;
  enum zsetby domZ_setby;

  unsigned nmodels;
  unsigned nseqs;
  unsigned n_past_msv;
  unsigned n_past_bias;
  unsigned n_past_vit;
  unsigned n_past_fwd;

  unsigned nhits;
  unsigned nreported;
  unsigned nincluded;
};

static_assert2(sizeof(unsigned) >= 4);

void h3r_stats_init(struct h3r_stats *);
int  h3r_stats_pack(struct h3r_stats const *, struct lio_writer *);
int  h3r_stats_unpack(struct h3r_stats *, struct lio_reader *);

#endif
