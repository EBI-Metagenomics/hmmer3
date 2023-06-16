#ifndef HMMD_STATS_H
#define HMMD_STATS_H

#include "hmmd/zsetby.h"
#include <stddef.h>
#include <stdint.h>

struct hmmd_stats
{
  double Z;
  double domZ;
  enum hmmd_zsetby Z_setby;
  enum hmmd_zsetby domZ_setby;

  uint64_t nmodels;
  uint64_t nseqs;
  uint64_t n_past_msv;
  uint64_t n_past_bias;
  uint64_t n_past_vit;
  uint64_t n_past_fwd;

  uint64_t nhits;
  uint64_t nreported;
  uint64_t nincluded;
  uint64_t *hit_offsets;
};

void h3client_hmmd_stats_init(struct hmmd_stats *);
void h3client_hmmd_stats_cleanup(struct hmmd_stats *);
int h3client_hmmd_stats_parse(struct hmmd_stats *stats,
                              unsigned char const **ptr,
                              unsigned char const *end);

#endif
