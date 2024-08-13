#include "hmmd_hit.h"
#include "defer_return.h"
#include "h3c_errno.h"
#include "hmmd_domain.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void h3client_hmmd_hit_init(struct hmmd_hit *hit)
{
  memset(hit, 0, sizeof(*hit));
}

void h3client_hmmd_hit_cleanup(struct hmmd_hit *hit)
{
  if (hit->name) free(hit->name);
  if (hit->acc) free(hit->acc);
  if (hit->desc) free(hit->desc);

  for (unsigned i = 0; i < hit->ndom; i++)
    h3client_hmmd_domain_cleanup(hit->dcl + i);

  free(hit->dcl);
  h3client_hmmd_hit_init(hit);
}

#define ACC_PRESENT (1 << 0)
#define DESC_PRESENT (1 << 1)

static int parse_strings(struct hmmd_hit *hit, uint8_t presence, size_t size,
                         unsigned char const **ptr)
{
  unsigned n = 1 + !!(presence & ACC_PRESENT) + !!(presence & DESC_PRESENT);
  if (!h3client_expect_n_strings(size, (char const *)*ptr, n))
    return H3C_EPARSE;

  int rc = 0;

  if ((rc = h3client_eatstr(&hit->name, ptr))) return rc;

  if (presence & ACC_PRESENT)
  {
    if ((rc = h3client_eatstr(&hit->acc, ptr))) return rc;
  }

  if (presence & DESC_PRESENT)
  {
    if ((rc = h3client_eatstr(&hit->desc, ptr))) return rc;
  }

  return 0;
}

int h3client_hmmd_hit_parse(struct hmmd_hit *hit, unsigned char const **ptr,
                            unsigned char const *end)
{
  int rc = 0;

  if (end < *ptr + 2 * sizeof(uint32_t)) defer_return(H3C_EPARSE);

  // Skips object size
  (void)h3client_eatu32(ptr);

  // Skips window_length for now
  (void)h3client_eati32(ptr);

  if (end < *ptr + 4 * sizeof(double) + 3 * sizeof(float))
    defer_return(H3C_EPARSE);
  hit->sortkey = h3client_eatf64(ptr);
  hit->score = h3client_eatf32(ptr);
  hit->pre_score = h3client_eatf32(ptr);
  hit->sum_score = h3client_eatf32(ptr);

  hit->lnP = h3client_eatf64(ptr);
  hit->pre_lnP = h3client_eatf64(ptr);
  hit->sum_lnP = h3client_eatf64(ptr);

  if (end < *ptr + 5 * sizeof(uint32_t) + sizeof(float))
    defer_return(H3C_EPARSE);
  hit->nexpected = h3client_eatf32(ptr);
  hit->nregions = h3client_eatu32(ptr);
  hit->nclustered = h3client_eatu32(ptr);
  hit->noverlaps = h3client_eatu32(ptr);
  hit->nenvelopes = h3client_eatu32(ptr);
  uint32_t ndom = h3client_eatu32(ptr);

  if (end < *ptr + 4 * sizeof(uint32_t) + 2 * sizeof(uint64_t))
    defer_return(H3C_EPARSE);
  hit->flags = h3client_eatu32(ptr);
  hit->nreported = h3client_eatu32(ptr);
  hit->nincluded = h3client_eatu32(ptr);
  hit->best_domain = h3client_eatu32(ptr);
  // Skips seqidx
  (void)h3client_eatu64(ptr);
  // Skips subseq_start for now
  (void)h3client_eatu64(ptr);

  if (end < *ptr + sizeof(uint8_t)) defer_return(H3C_EPARSE);
  uint8_t presence = h3client_eatu8(ptr);

  if ((rc = parse_strings(hit, presence, (end - *ptr), ptr))) defer_return(rc);

  if (ndom > hit->ndom)
  {
    struct hmmd_domain *dcl = realloc(hit->dcl, ndom * sizeof(*hit->dcl));
    if (!dcl) defer_return(H3C_ENOMEM);

    hit->dcl = dcl;
    for (uint32_t i = hit->ndom; i < ndom; i++)
    {
      h3client_hmmd_domain_init(hit->dcl + i);
      ++hit->ndom;
    }
  }
  else
  {
    for (uint32_t i = ndom; i < hit->ndom; i++)
      h3client_hmmd_domain_cleanup(hit->dcl + i);
    hit->ndom = ndom;
  }

  for (uint32_t i = 0; i < hit->ndom; i++)
  {
    if ((rc = h3client_hmmd_domain_parse(hit->dcl + i, ptr, end))) defer_return(rc);
  }

  return 0;

defer:
  return rc;
}
