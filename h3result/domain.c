#include "domain.h"
#include "alidisplay.h"
#include "expect.h"
#include "lip/lip.h"
#include "rc.h"
#include "reallocf.h"
#include <stdlib.h>
#include <string.h>

void h3result_domain_init(struct domain *dom)
{
  memset(dom, 0, offsetof(struct domain, ad));
  h3result_alidisplay_init(&dom->ad);
}

static int grow_scores(struct domain *dom, unsigned size)
{
  size_t sz = size * sizeof(*dom->pos_score);
  if (!(dom->pos_score = h3result_reallocf(dom->pos_score, sz)))
  {
    h3result_domain_cleanup(dom);
    return H3RESULT_ENOMEM;
  }
  return 0;
}

static void shrink_scores(struct domain *dom, unsigned size)
{
  dom->pos_score_size = size;
}

int h3result_domain_setup(struct domain *dom, unsigned scores_size)
{
  if (dom->pos_score_size < scores_size) return grow_scores(dom, scores_size);
  shrink_scores(dom, scores_size);
  return 0;
}

void h3result_domain_cleanup(struct domain *dom)
{
  free(dom->pos_score);
  dom->pos_score = NULL;
  dom->pos_score_size = 0;
  h3result_alidisplay_cleanup(&dom->ad);
}

int h3result_domain_pack(struct domain const *dom, struct lip_file *f)
{
  lip_write_array_size(f, 14);

  lip_write_int(f, dom->ienv);
  lip_write_int(f, dom->jenv);

  lip_write_int(f, dom->iali);
  lip_write_int(f, dom->jali);

  lip_write_float(f, dom->envsc);
  lip_write_float(f, dom->domcorrection);
  lip_write_float(f, dom->dombias);
  lip_write_float(f, dom->oasc);
  lip_write_float(f, dom->bitscore);
  lip_write_float(f, dom->lnP);

  lip_write_bool(f, dom->is_reported);
  lip_write_bool(f, dom->is_included);

  lip_write_array_size(f, dom->pos_score_size);
  for (unsigned long i = 0; i < dom->pos_score_size; i++)
    lip_write_float(f, dom->pos_score[i]);

  lip_write_map_size(f, 1);
  lip_write_cstr(f, "alidisplay");

  return h3result_alidisplay_pack(&dom->ad, f);
}

int h3result_domain_unpack(struct domain *dom, struct lip_file *f)
{
  int rc = H3RESULT_EUNPACK;

  if (!h3result_expect_array_size(f, 14)) goto cleanup;

  lip_read_int(f, &dom->ienv);
  lip_read_int(f, &dom->jenv);

  lip_read_int(f, &dom->iali);
  lip_read_int(f, &dom->jali);

  lip_read_float(f, &dom->envsc);
  lip_read_float(f, &dom->domcorrection);
  lip_read_float(f, &dom->dombias);
  lip_read_float(f, &dom->oasc);
  lip_read_float(f, &dom->bitscore);
  lip_read_float(f, &dom->lnP);

  lip_read_bool(f, &dom->is_reported);
  lip_read_bool(f, &dom->is_included);

  unsigned size = 0;
  lip_read_array_size(f, &size);
  if ((rc = h3result_domain_setup(dom, size))) goto cleanup;

  for (unsigned long i = 0; i < dom->pos_score_size; i++)
    lip_read_float(f, dom->pos_score + i);

  if (!h3result_expect_map_size(f, 1)) goto cleanup;
  if (!h3result_expect_key(f, "alidisplay")) goto cleanup;

  if ((rc = h3result_alidisplay_unpack(&dom->ad, f))) goto cleanup;

  return 0;

cleanup:
  h3result_domain_cleanup(dom);
  return rc;
}
