#include "answer.h"
#include "buff.h"
#include "domain.h"
#include "h3client/errno.h"
#include "h3client/result.h"
#include "hit.h"
#include "hmmd/hmmd.h"
#include "result.h"
#include "stats.h"
#include "tophits.h"
#include "utils.h"
#include "zc.h"
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

  if (ptr != end) rc = H3CLIENT_EPARSE;

cleanup:
  return rc;
}

int h3client_answer_parse_error(struct answer *x)
{
  x->errnum = x->status.value.status;

  size_t msg_size = x->status.value.msg_size;
  char *errstr = realloc(x->errstr, msg_size + 1);
  if (!errstr) return H3CLIENT_ENOMEM;
  memcpy(errstr, x->buff->data, msg_size);
  errstr[msg_size] = '\0';
  x->errstr = errstr;

  return 0;
}

#define STRXDUP(D, S) (D = h3client_strxdup((D), (S)))

#define CHECK_OVERFLOW(var, val)                                               \
  do                                                                           \
  {                                                                            \
    if ((var) > (val))                                                         \
    {                                                                          \
      rc = H3CLIENT_EOUTRANGE;                                                 \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0);

static int copy_alidisplay(struct alidisplay *dst,
                           struct hmmd_alidisplay const *src)
{
  int rc = H3CLIENT_ENOMEM;

  dst->presence = src->presence;

  if (!STRXDUP(dst->rfline, src->rfline)) goto cleanup;
  if (!STRXDUP(dst->mmline, src->mmline)) goto cleanup;
  if (!STRXDUP(dst->csline, src->csline)) goto cleanup;
  if (!STRXDUP(dst->model, src->model)) goto cleanup;
  if (!STRXDUP(dst->mline, src->mline)) goto cleanup;
  if (!STRXDUP(dst->aseq, src->aseq)) goto cleanup;
  if (!STRXDUP(dst->ntseq, src->ntseq)) goto cleanup;
  if (!STRXDUP(dst->ppline, src->ppline)) goto cleanup;
  dst->N = src->N;

  if (!STRXDUP(dst->hmmname, src->hmmname)) goto cleanup;
  if (!STRXDUP(dst->hmmacc, src->hmmacc)) goto cleanup;
  if (!STRXDUP(dst->hmmdesc, src->hmmdesc)) goto cleanup;
  dst->hmmfrom = src->hmmfrom;
  dst->hmmto = src->hmmto;
  dst->M = src->M;

  CHECK_OVERFLOW(src->sqfrom, UINT32_MAX);
  CHECK_OVERFLOW(src->sqto, UINT32_MAX);
  CHECK_OVERFLOW(src->L, UINT32_MAX);

  if (!STRXDUP(dst->sqname, src->sqname)) goto cleanup;
  if (!STRXDUP(dst->sqacc, src->sqacc)) goto cleanup;
  if (!STRXDUP(dst->sqdesc, src->sqdesc)) goto cleanup;
  dst->sqfrom = src->sqfrom;
  dst->sqto = src->sqto;
  dst->L = src->L;

  return 0;

cleanup:
  h3client_alidisplay_cleanup(dst);
  return rc;
}

static int copy_domain(struct domain *dst, struct hmmd_domain const *src)
{
  int rc = h3client_domain_setup(dst, src->scores_size);
  if (rc) return rc;

  CHECK_OVERFLOW(src->ienv, UINT32_MAX);
  CHECK_OVERFLOW(src->jenv, UINT32_MAX);
  CHECK_OVERFLOW(src->iali, UINT32_MAX);
  CHECK_OVERFLOW(src->jali, UINT32_MAX);

  dst->ienv = src->ienv;
  dst->jenv = src->jenv;
  dst->iali = src->iali;
  dst->jali = src->jali;
  dst->envsc = src->envsc;
  dst->domcorrection = src->domcorrection;
  dst->dombias = src->dombias;
  dst->oasc = src->oasc;
  dst->bitscore = src->bitscore;
  dst->lnP = src->lnP;
  dst->is_reported = src->is_reported;
  dst->is_included = src->is_included;

  CHECK_OVERFLOW(src->scores_size, UINT32_MAX);
  for (unsigned i = 0; i < dst->pos_score_size; ++i)
    dst->pos_score[i] = src->pos_score[i];

  if ((rc = copy_alidisplay(&dst->ad, &src->ad))) goto cleanup;

  return 0;

cleanup:
  h3client_domain_cleanup(dst);
  return rc;
}

static int copy_hit(struct hit *dst, struct hmmd_hit const *src)
{
  int rc = h3client_hit_setup(dst, src->ndom);
  if (rc) return rc;

  if (!STRXDUP(dst->name, src->name)) goto cleanup;
  if (!STRXDUP(dst->acc, src->acc)) goto cleanup;
  if (!STRXDUP(dst->desc, src->desc)) goto cleanup;

  dst->sortkey = src->sortkey;

  dst->score = src->score;
  dst->pre_score = src->pre_score;
  dst->sum_score = src->sum_score;

  dst->lnP = src->lnP;
  dst->pre_lnP = src->pre_lnP;
  dst->sum_lnP = src->sum_lnP;

  dst->nexpected = src->nexpected;
  dst->nregions = src->nregions;
  dst->nclustered = src->nclustered;
  dst->noverlaps = src->noverlaps;
  dst->nenvelopes = src->nenvelopes;

  dst->flags = src->flags;
  dst->nreported = src->nreported;
  dst->nincluded = src->nincluded;
  dst->best_domain = src->best_domain;

  for (unsigned i = 0; i < dst->ndomains; ++i)
  {
    if ((rc = copy_domain(dst->domains + i, src->dcl + i))) goto cleanup;
  }

  return 0;

cleanup:
  h3client_hit_cleanup(dst);
  return rc;
}

static int copy_tophits(struct tophits *dst, struct hmmd_tophits const *src)
{
  int rc = h3client_tophits_setup(dst, src->nhits);
  if (rc) return rc;

  dst->nreported = src->nreported;
  dst->nincluded = src->nincluded;
  dst->is_sorted_by_sortkey = src->is_sorted_by_sortkey;
  dst->is_sorted_by_seqidx = src->is_sorted_by_seqidx;

  for (unsigned long i = 0; i < src->nhits; ++i)
  {
    if ((rc = copy_hit(dst->hits + i, src->hit[i]))) goto cleanup;
  }

  return 0;

cleanup:
  h3client_tophits_cleanup(dst);
  return rc;
}

static int copy_stats(struct stats *dst, struct hmmd_stats const *src)
{
  int rc = 0;

  dst->Z = src->Z;
  dst->domZ = src->domZ;

  dst->Z_setby = (enum zsetby)src->Z_setby;
  dst->domZ_setby = (enum zsetby)src->domZ_setby;

  CHECK_OVERFLOW(src->nmodels, UINT32_MAX);
  CHECK_OVERFLOW(src->nseqs, UINT32_MAX);
  CHECK_OVERFLOW(src->n_past_msv, UINT32_MAX);
  CHECK_OVERFLOW(src->n_past_bias, UINT32_MAX);
  CHECK_OVERFLOW(src->n_past_vit, UINT32_MAX);
  CHECK_OVERFLOW(src->n_past_fwd, UINT32_MAX);

  dst->nmodels = src->nmodels;
  dst->nseqs = src->nseqs;
  dst->n_past_msv = src->n_past_msv;
  dst->n_past_bias = src->n_past_bias;
  dst->n_past_vit = src->n_past_vit;
  dst->n_past_fwd = src->n_past_fwd;

  CHECK_OVERFLOW(src->nhits, UINT32_MAX);
  CHECK_OVERFLOW(src->nreported, UINT32_MAX);
  CHECK_OVERFLOW(src->nincluded, UINT32_MAX);

  dst->nhits = src->nhits;
  dst->nreported = src->nreported;
  dst->nincluded = src->nincluded;

cleanup:
  return rc;
}

int h3client_answer_copy(struct answer *x, struct h3client_result *y)
{
  y->errnum = x->errnum;
  y->errstr = x->errstr;
  x->errstr = NULL;
  if (y->errnum) return 0;
  copy_stats(&y->stats, &x->stats);
  return copy_tophits(&y->tophits, &x->tophits);
}
