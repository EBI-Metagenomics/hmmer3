#include "h3r_domain.h"
#include "bsd.h"
#include "expect.h"
#include "h3r_alidisplay.h"
#include "lio.h"
#include "rc.h"
#include "read.h"
#include "write.h"
#include <stdlib.h>
#include <string.h>

static inline void unset(struct domain *x)
{
  x->ienv           = 0;
  x->jenv           = 0;
  x->iali           = 0;
  x->jali           = 0;
  x->envsc          = 0;
  x->domcorrection  = 0;
  x->dombias        = 0;
  x->oasc           = 0;
  x->bitscore       = 0;
  x->lnP            = 0;
  x->is_reported    = false;
  x->is_included    = false;
  x->pos_score_size = 0;
  x->pos_score      = NULL;
}

int h3r_domain_init(struct domain *x)
{
  unset(x);
  return h3r_alidisplay_init(&x->ad);
}

static int grow_scores(struct domain *x, unsigned size)
{
  size_t sz = size * sizeof(*x->pos_score);
  if (!(x->pos_score = bsd_reallocf(x->pos_score, sz)))
  {
    h3r_domain_cleanup(x);
    return H3R_ENOMEM;
  }
  return 0;
}

static void shrink_scores(struct domain *x, unsigned size)
{
  x->pos_score_size = size;
}

int h3r_domain_setup(struct domain *x, unsigned scores_size)
{
  if (x->pos_score_size < scores_size) return grow_scores(x, scores_size);
  shrink_scores(x, scores_size);
  return 0;
}

void h3r_domain_cleanup(struct domain *x)
{
  free(x->pos_score);
  unset(x);
  h3r_alidisplay_cleanup(&x->ad);
}

int h3r_domain_pack(struct domain const *x, struct lio_writer *f)
{
  if (write_array(f, 14))                return H3R_EPACK;

  if (write_int(f, x->ienv))             return H3R_EPACK;
  if (write_int(f, x->jenv))             return H3R_EPACK;

  if (write_int(f, x->iali))             return H3R_EPACK;
  if (write_int(f, x->jali))             return H3R_EPACK;

  if (write_float(f, x->envsc))          return H3R_EPACK;
  if (write_float(f, x->domcorrection))  return H3R_EPACK;
  if (write_float(f, x->dombias))        return H3R_EPACK;
  if (write_float(f, x->oasc))           return H3R_EPACK;
  if (write_float(f, x->bitscore))       return H3R_EPACK;
  if (write_float(f, x->lnP))            return H3R_EPACK;

  if (write_bool(f, x->is_reported))     return H3R_EPACK;
  if (write_bool(f, x->is_included))     return H3R_EPACK;

  if (write_array(f, x->pos_score_size)) return H3R_EPACK;
  for (unsigned long i = 0; i < x->pos_score_size; i++)
  {
    if (write_float(f, x->pos_score[i])) return H3R_EPACK;
  }

  if (write_map(f, 1))                   return H3R_EPACK;
  if (write_cstring(f, "alidisplay"))    return H3R_EPACK;

  return h3r_alidisplay_pack(&x->ad, f);
}

int h3r_domain_unpack(struct domain *x, struct lio_reader *f)
{
  int rc = 0;

  if (expect_array(f, 14))                     return H3R_EUNPACK;

  if (read_int(f, &x->ienv))                   return H3R_EUNPACK;
  if (read_int(f, &x->jenv))                   return H3R_EUNPACK;

  if (read_int(f, &x->iali))                   return H3R_EUNPACK;
  if (read_int(f, &x->jali))                   return H3R_EUNPACK;

  if (read_float(f, &x->envsc))                return H3R_EUNPACK;
  if (read_float(f, &x->domcorrection))        return H3R_EUNPACK;
  if (read_float(f, &x->dombias))              return H3R_EUNPACK;
  if (read_float(f, &x->oasc))                 return H3R_EUNPACK;
  if (read_float(f, &x->bitscore))             return H3R_EUNPACK;
  if (read_float(f, &x->lnP))                  return H3R_EUNPACK;

  if (read_bool(f, &x->is_reported))           return H3R_EUNPACK;
  if (read_bool(f, &x->is_included))           return H3R_EUNPACK;

  unsigned size = 0;
  if (read_array(f, &size))                    return H3R_EUNPACK;
  if ((rc = h3r_domain_setup(x, size)))        return H3R_EUNPACK;

  for (unsigned long i = 0; i < x->pos_score_size; i++)
  {
    if (read_float(f, x->pos_score + i))       return H3R_EUNPACK;
  }

  if (expect_map(f, 1))                        return H3R_EUNPACK;
  if (expect_key(f, "alidisplay"))             return H3R_EUNPACK;

  if ((rc = h3r_alidisplay_unpack(&x->ad, f))) return H3R_EUNPACK;

  return rc;
}
