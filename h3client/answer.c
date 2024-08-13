#include "answer.h"
#include "buff.h"
#include "defer_return.h"
#include "h3c_errno.h"
#include "h3r_alidisplay.h"
#include "h3r_domain.h"
#include "h3r_hit.h"
#include "h3r_result.h"
#include "h3r_stats.h"
#include "h3r_tophits.h"
#include "hmmd_hmmd.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 2048

struct answer
{
  struct
  {
    unsigned char data[HMMD_STATUS_PACK_SIZE];
    struct hmmd_status value;
  } status;

  struct buff *buff;
  int errnum;
  char *errstr;
  struct hmmd_stats stats;
  struct hmmd_tophits tophits;
};

struct answer *h3client_answer_new(void)
{
  struct answer *ans = malloc(sizeof(*ans));
  if (!ans) return 0;

  ans->status.data[0] = '\0';
  h3client_hmmd_status_init(&ans->status.value);

  if (!(ans->buff = h3client_buff_new(BUFF_SIZE)))
  {
    free(ans);
    return 0;
  }

  ans->errnum = 0;
  ans->errstr = NULL;
  h3client_hmmd_stats_init(&ans->stats);
  h3client_hmmd_tophits_init(&ans->tophits);
  return ans;
}

void h3client_answer_del(struct answer const *ans)
{
  h3client_buff_del(ans->buff);
  if (ans->errstr) free((void *)ans->errstr);
  h3client_hmmd_stats_cleanup((struct hmmd_stats *)&ans->stats);
  h3client_hmmd_tophits_cleanup((struct hmmd_tophits *)&ans->tophits);
  free((void *)ans);
}

unsigned char *h3client_answer_status_data(struct answer *ans)
{
  return ans->status.data;
}

size_t h3client_answer_status_size(void) { return HMMD_STATUS_PACK_SIZE; }

struct hmmd_status const *h3client_answer_status_parse(struct answer *ans)
{
  size_t size = 0;
  h3client_hmmd_status_parse(&ans->status.value, &size, ans->status.data);
  return h3client_answer_status(ans);
}

struct hmmd_status const *h3client_answer_status(struct answer const *ans)
{
  return &ans->status.value;
}

int h3client_answer_setup_size(struct answer *ans, size_t size)
{
  ans->buff->size = size;
  return h3client_buff_ensure(&ans->buff, size);
}

unsigned char *h3client_answer_data(struct answer *ans)
{
  return ans->buff->data;
}

int h3client_answer_parse(struct answer *ans)
{
  int rc = 0;

  unsigned char const *ptr = ans->buff->data;
  unsigned char const *end = ptr + ans->buff->size;
  if ((rc = h3client_hmmd_stats_parse(&ans->stats, &ptr, end))) goto cleanup;

  rc = h3client_hmmd_tophits_setup(&ans->tophits, &ptr, end, ans->stats.nhits,
                                   ans->stats.nreported, ans->stats.nincluded);
  if (rc) goto cleanup;

  if (ptr != end) rc = H3C_EPARSE;

cleanup:
  return rc;
}

int h3client_answer_parse_error(struct answer *x)
{
  x->errnum = x->status.value.status;

  size_t msg_size = x->status.value.msg_size;
  char *errstr = realloc(x->errstr, msg_size + 1);
  if (!errstr) return H3C_ENOMEM;
  memcpy(errstr, x->buff->data, msg_size);
  errstr[msg_size] = '\0';
  x->errstr = errstr;

  return 0;
}

#define STRXDUP(D, S) (D = h3client_strxdup((D), (S)))

static int copy_alidisplay(struct h3r_alidisplay *dst,
                           struct hmmd_alidisplay const *src)
{
  dst->presence = src->presence;

  if (!STRXDUP(dst->rfline, src->rfline)) return H3C_ENOMEM;
  if (!STRXDUP(dst->mmline, src->mmline)) return H3C_ENOMEM;
  if (!STRXDUP(dst->csline, src->csline)) return H3C_ENOMEM;
  if (!STRXDUP(dst->model, src->model))   return H3C_ENOMEM;
  if (!STRXDUP(dst->mline, src->mline))   return H3C_ENOMEM;
  if (!STRXDUP(dst->aseq, src->aseq))     return H3C_ENOMEM;
  if (!STRXDUP(dst->ntseq, src->ntseq))   return H3C_ENOMEM;
  if (!STRXDUP(dst->ppline, src->ppline)) return H3C_ENOMEM;
  dst->N = src->N;

  if (!STRXDUP(dst->hmmname, src->hmmname)) return H3C_ENOMEM;
  if (!STRXDUP(dst->hmmacc, src->hmmacc))   return H3C_ENOMEM;
  if (!STRXDUP(dst->hmmdesc, src->hmmdesc)) return H3C_ENOMEM;
  dst->hmmfrom = src->hmmfrom;
  dst->hmmto = src->hmmto;
  dst->M = src->M;

  if (src->sqfrom > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->sqto   > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->L      > UINT32_MAX) return H3C_EOUTRANGE;

  if (!STRXDUP(dst->sqname, src->sqname)) return H3C_ENOMEM;
  if (!STRXDUP(dst->sqacc, src->sqacc))   return H3C_ENOMEM;
  if (!STRXDUP(dst->sqdesc, src->sqdesc)) return H3C_ENOMEM;
  dst->sqfrom = src->sqfrom;
  dst->sqto = src->sqto;
  dst->L = src->L;

  return 0;
}

static int copy_domain(struct h3r_domain *dst, struct hmmd_domain const *src)
{
  int rc = h3r_domain_setup(dst, src->scores_size);
  if (rc) return rc;

  if (src->ienv > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->jenv > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->iali > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->jali > UINT32_MAX) defer_return(H3C_EOUTRANGE);

  dst->ienv          = src->ienv;
  dst->jenv          = src->jenv;
  dst->iali          = src->iali;
  dst->jali          = src->jali;
  dst->envsc         = src->envsc;
  dst->domcorrection = src->domcorrection;
  dst->dombias       = src->dombias;
  dst->oasc          = src->oasc;
  dst->bitscore      = src->bitscore;
  dst->lnP           = src->lnP;
  dst->is_reported   = src->is_reported;
  dst->is_included   = src->is_included;

  if (src->scores_size > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  for (unsigned i = 0; i < dst->pos_score_size; ++i)
    dst->pos_score[i] = src->pos_score[i];

  if ((rc = copy_alidisplay(&dst->ad, &src->ad))) defer_return(rc);

  return 0;

defer:
  h3r_domain_cleanup(dst);
  return rc;
}

static int copy_hit(struct h3r_hit *dst, struct hmmd_hit const *src)
{
  int rc = h3r_hit_setup(dst, src->ndom);
  if (rc) return rc;

  if (!STRXDUP(dst->name, src->name)) defer_return(H3C_ENOMEM);
  if (!STRXDUP(dst->acc, src->acc))   defer_return(H3C_ENOMEM);
  if (!STRXDUP(dst->desc, src->desc)) defer_return(H3C_ENOMEM);

  dst->sortkey     = src->sortkey;

  dst->score       = src->score;
  dst->pre_score   = src->pre_score;
  dst->sum_score   = src->sum_score;

  dst->lnP         = src->lnP;
  dst->pre_lnP     = src->pre_lnP;
  dst->sum_lnP     = src->sum_lnP;

  dst->nexpected   = src->nexpected;
  dst->nregions    = src->nregions;
  dst->nclustered  = src->nclustered;
  dst->noverlaps   = src->noverlaps;
  dst->nenvelopes  = src->nenvelopes;

  dst->flags       = src->flags;
  dst->nreported   = src->nreported;
  dst->nincluded   = src->nincluded;
  dst->best_domain = src->best_domain;

  for (unsigned i = 0; i < dst->ndomains; ++i)
  {
    if ((rc = copy_domain(dst->domains + i, src->dcl + i))) defer_return(rc);
  }

  return 0;

defer:
  h3r_hit_cleanup(dst);
  return rc;
}

static int copy_tophits(struct h3r_tophits *dst, struct hmmd_tophits const *src)
{
  int rc = h3r_tophits_setup(dst, src->nhits);
  if (rc) return rc;

  dst->nreported            = src->nreported;
  dst->nincluded            = src->nincluded;
  dst->is_sorted_by_sortkey = src->is_sorted_by_sortkey;
  dst->is_sorted_by_seqidx  = src->is_sorted_by_seqidx;

  for (unsigned long i = 0; i < src->nhits; ++i)
  {
    if ((rc = copy_hit(dst->hits + i, src->hit[i]))) defer_return(rc);
  }

  return 0;

defer:
  h3r_tophits_cleanup(dst);
  return rc;
}

static int copy_stats(struct h3r_stats *dst, struct hmmd_stats const *src)
{
  int rc = 0;

  dst->Z = src->Z;
  dst->domZ = src->domZ;

  dst->Z_setby    = (enum h3r_zsetby)src->Z_setby;
  dst->domZ_setby = (enum h3r_zsetby)src->domZ_setby;

  if (src->nmodels     > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->nseqs       > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->n_past_msv  > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->n_past_bias > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->n_past_vit  > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->n_past_fwd  > UINT32_MAX) defer_return(H3C_EOUTRANGE);

  dst->nmodels = src->nmodels;
  dst->nseqs = src->nseqs;
  dst->n_past_msv = src->n_past_msv;
  dst->n_past_bias = src->n_past_bias;
  dst->n_past_vit = src->n_past_vit;
  dst->n_past_fwd = src->n_past_fwd;

  if (src->nhits     > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->nreported > UINT32_MAX) defer_return(H3C_EOUTRANGE);
  if (src->nincluded > UINT32_MAX) defer_return(H3C_EOUTRANGE);

  dst->nhits = src->nhits;
  dst->nreported = src->nreported;
  dst->nincluded = src->nincluded;

defer:
  return rc;
}

int h3client_answer_copy(struct answer *x, struct h3r *y)
{
  copy_stats(&y->stats, &x->stats);
  return copy_tophits(&y->tophits, &x->tophits);
}
