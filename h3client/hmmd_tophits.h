#ifndef HMMD_TOPHITS_H
#define HMMD_TOPHITS_H

#include <stdbool.h>
#include <stdint.h>

struct hmmd_hit;

struct hmmd_tophits
{
  struct hmmd_hit **hit;
  struct hmmd_hit *unsrt;
  uint64_t nhits;
  uint64_t nreported;
  uint64_t nincluded;
  bool is_sorted_by_sortkey;
  bool is_sorted_by_seqidx;
};

void h3client_hmmd_tophits_init(struct hmmd_tophits *);
int h3client_hmmd_tophits_setup(struct hmmd_tophits *,
                                unsigned char const **ptr,
                                unsigned char const *end, uint64_t nhits,
                                uint64_t nreported, uint64_t nincluded);
void h3client_hmmd_tophits_cleanup(struct hmmd_tophits *);

#endif
