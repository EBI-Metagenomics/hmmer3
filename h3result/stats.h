#ifndef H3RESULT_STATS_H
#define H3RESULT_STATS_H

#include "static_assert2.h"
#include "zsetby.h"
#include <stdint.h>

struct stats;
struct lio_writer;
struct lio_reader;

struct stats
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

void h3result_stats_init(struct stats *);
int h3result_stats_pack(struct stats const *, struct lio_writer *);
int h3result_stats_unpack(struct stats *, struct lio_reader *);

#endif
