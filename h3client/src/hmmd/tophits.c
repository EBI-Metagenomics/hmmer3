#include "hmmd/tophits.h"
#include "h3client/errno.h"
#include "hmmd/domain.h"
#include "hmmd/hit.h"
#include "zc.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void h3client_hmmd_tophits_init(struct hmmd_tophits *th)
{
  memset(th, 0, sizeof(*th));
  th->hit = 0;
  th->unsrt = 0;
  th->is_sorted_by_sortkey = true;
}

static int grow(struct hmmd_tophits *th, uint64_t nhits)
{
  int rc = 0;

  if (!(th->hit = zc_reallocf(th->hit, sizeof(*th->hit) * nhits)))
  {
    rc = H3CLIENT_ENOMEM;
    goto cleanup;
  }

  size_t sz = nhits * sizeof(*th->unsrt);
  struct hmmd_hit *hits = realloc(th->unsrt, sz);
  if (!hits)
  {
    rc = H3CLIENT_ENOMEM;
    goto cleanup;
  }
  th->unsrt = hits;

  for (uint64_t i = th->nhits; i < nhits; ++i)
  {
    h3client_hmmd_hit_init(th->unsrt + i);
    ++th->nhits;
  }

  return 0;

cleanup:
  h3client_hmmd_tophits_cleanup(th);
  return rc;
}

static void shrink(struct hmmd_tophits *th, uint64_t nhits)
{
  for (uint64_t i = nhits; i < th->nhits; ++i)
  {
    th->hit[i] = 0;
    h3client_hmmd_hit_cleanup(th->unsrt + i);
  }

  th->nhits = nhits;
}

int h3client_hmmd_tophits_setup(struct hmmd_tophits *th,
                                unsigned char const **ptr,
                                unsigned char const *end, uint64_t nhits,
                                uint64_t nreported, uint64_t nincluded)
{
  int rc = 0;

  if (th->nhits < nhits)
    rc = grow(th, nhits);
  else
    shrink(th, nhits);

  if (rc) goto cleanup;

  th->nreported = nreported;
  th->nincluded = nincluded;
  th->is_sorted_by_seqidx = false;
  th->is_sorted_by_sortkey = true;

  for (uint64_t i = 0; i < nhits; ++i)
  {
    if ((rc = h3client_hmmd_hit_parse(th->unsrt + i, ptr, end))) goto cleanup;
    th->hit[i] = th->unsrt + i;
  }

  return 0;

cleanup:
  h3client_hmmd_tophits_cleanup(th);
  return rc;
}

void h3client_hmmd_tophits_cleanup(struct hmmd_tophits *th)
{
  for (uint64_t i = 0; i < th->nhits; ++i)
  {
    h3client_hmmd_hit_cleanup(th->unsrt + i);
  }
  free(th->hit);
  free(th->unsrt);
  h3client_hmmd_tophits_init(th);
}
