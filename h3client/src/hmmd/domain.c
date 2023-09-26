#include "hmmd/domain.h"
#include "compiler.h"
#include "h3client/errno.h"
#include "hmmd/alidisplay.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void h3client_hmmd_domain_init(struct hmmd_domain *dom)
{
  memset(dom, 0, offsetof(struct hmmd_domain, ad));
  h3client_hmmd_alidisplay_init(&dom->ad);
}

void h3client_hmmd_domain_cleanup(struct hmmd_domain *dom)
{
  if (dom->pos_score) free(dom->pos_score);
  h3client_hmmd_alidisplay_cleanup(&dom->ad);
  h3client_hmmd_domain_init(dom);
}

static_assert(sizeof(float) == 4, "sizeof(float) == 4");
static_assert(sizeof(double) == 8, "sizeof(double) == 8");

int h3client_hmmd_domain_parse(struct hmmd_domain *dom,
                               unsigned char const **ptr,
                               unsigned char const *end)
{
  int rc = 0;

  ESCAPE_OVERRUN(rc, *ptr, end, sizeof(uint32_t) + 6 * sizeof(uint64_t));

  // Skips object size
  (void)h3client_eatu32(ptr);
  dom->ienv = h3client_eatu64(ptr);
  dom->jenv = h3client_eatu64(ptr);
  dom->iali = h3client_eatu64(ptr);
  dom->jali = h3client_eatu64(ptr);

  // Skips iorf and jorf.
  (void)h3client_eati64(ptr);
  (void)h3client_eati64(ptr);

  ESCAPE_OVERRUN(rc, *ptr, end, 5 * sizeof(float) + sizeof(double));
  ESCAPE_OVERRUN(rc, *ptr, end, 3 * sizeof(uint32_t));

  dom->envsc = h3client_eatf32(ptr);
  dom->domcorrection = h3client_eatf32(ptr);
  dom->dombias = h3client_eatf32(ptr);
  dom->oasc = h3client_eatf32(ptr);
  dom->bitscore = h3client_eatf32(ptr);
  dom->lnP = h3client_eatf64(ptr);
  dom->is_reported = h3client_eatu32(ptr);
  dom->is_included = h3client_eatu32(ptr);

  uint32_t scores_size = h3client_eatu32(ptr);

  if (scores_size > dom->scores_size)
  {
    size_t size = scores_size * sizeof(float);
    float *scores = realloc(dom->pos_score, size);
    if (!scores)
    {
      rc = H3CLIENT_EPARSE;
      goto cleanup;
    }
    dom->pos_score = scores;
    dom->scores_size = scores_size;
  }
  else
    dom->scores_size = scores_size;

  ESCAPE_OVERRUN(rc, *ptr, end, dom->scores_size * sizeof(float));
  for (uint32_t i = 0; i < dom->scores_size; i++)
    dom->pos_score[i] = h3client_eatf32(ptr);

  if ((rc = h3client_hmmd_alidisplay_parse(&dom->ad, ptr, end))) goto cleanup;

  return 0;

cleanup:
  h3client_hmmd_domain_cleanup(dom);
  return rc;
}
