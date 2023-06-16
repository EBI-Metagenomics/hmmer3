#ifndef HMMD_HIT_H
#define HMMD_HIT_H

#include <stddef.h>
#include <stdint.h>

struct hmmd_domain;

struct hmmd_hit
{
  char *name;
  char *acc;
  char *desc;
  // The following members seem to be receiving
  // random numbers from daemon end. Skip them.
  // int window_length;
  double sortkey;

  float score;
  float pre_score;
  float sum_score;

  double lnP;
  double pre_lnP;
  double sum_lnP;

  float nexpected;
  uint32_t nregions;
  uint32_t nclustered;
  uint32_t noverlaps;
  uint32_t nenvelopes;
  uint32_t ndom;

  uint32_t flags;
  uint32_t nreported;
  uint32_t nincluded;
  uint32_t best_domain;

  // Skip it.
  // uint64_t seqidx;
  // The following member seems to be receiving
  // random numbers from daemon end. Skip it.
  // uint64_t subseq_start;

  struct hmmd_domain *dcl;
};

void h3client_hmmd_hit_init(struct hmmd_hit *);
void h3client_hmmd_hit_cleanup(struct hmmd_hit *);

int h3client_hmmd_hit_parse(struct hmmd_hit *hit, unsigned char const **ptr,
                            unsigned char const *end);

#endif
