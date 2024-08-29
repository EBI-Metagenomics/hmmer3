#include "body.h"
#include "posix.h"
#include "bsd.h"
#include "defer_return.h"
#include "h3c_errnum.h"
#include "h3r_domain.h"
#include "h3r_hit.h"
#include "h3r_result.h"
#include "h3r_stats.h"
#include "parse.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum zsetby
{
  ZSETBY_NTARGETS = 0,
  ZSETBY_OPTION   = 1,
  ZSETBY_FILEINFO = 2
};

struct stats
{
  double Z;
  double domZ;
  enum zsetby Z_setby;
  enum zsetby domZ_setby;

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

void stats_init(struct stats *x)
{
  x->Z          = 0;
  x->domZ       = 0;
  x->Z_setby    = 0;
  x->domZ_setby = 0;

  x->nmodels     = 0;
  x->nseqs       = 0;
  x->n_past_msv  = 0;
  x->n_past_bias = 0;
  x->n_past_vit  = 0;
  x->n_past_fwd  = 0;

  x->nhits       = 0;
  x->nreported   = 0;
  x->nincluded   = 0;
  x->hit_offsets = NULL;
}

void stats_cleanup(struct stats *x)
{
  free(x->hit_offsets);
  stats_init(x);
}

static int stats_parse(struct stats *x, unsigned char const *head,
                       unsigned char const *tail, size_t *size)
{
  unsigned char const *old_head = head;
  if (tail < head + 14 * 8 + 2) return H3C_EPARSE;


  (void)parse_double(head); head += 8; // skip elapsed
  (void)parse_double(head); head += 8; // skip user
  (void)parse_double(head); head += 8; // skip sys

  x->Z    = parse_double(head); head += 8;
  x->domZ = parse_double(head); head += 8;

  x->Z_setby    = parse_flag(head); head += 1;
  x->domZ_setby = parse_flag(head); head += 1;

  if (x->Z_setby    > ZSETBY_FILEINFO) return H3C_EPARSE;
  if (x->domZ_setby > ZSETBY_FILEINFO) return H3C_EPARSE;

  x->nmodels     = parse_uint64(head); head += 8;
  x->nseqs       = parse_uint64(head); head += 8;
  x->n_past_msv  = parse_uint64(head); head += 8;
  x->n_past_bias = parse_uint64(head); head += 8;
  x->n_past_vit  = parse_uint64(head); head += 8;
  x->n_past_fwd  = parse_uint64(head); head += 8;
  x->nhits       = parse_uint64(head); head += 8;
  x->nreported   = parse_uint64(head); head += 8;
  x->nincluded   = parse_uint64(head); head += 8;

  if (tail < head + x->nhits * 8) return H3C_EPARSE;

  uint64_t hit_offset0     = parse_uint64(head); head += 8;
  size_t   hit_offset_size = x->nhits * sizeof(uint64_t);

  if (hit_offset_size == UINT64_MAX && x->nhits > 0) return H3C_EPARSE;

  x->hit_offsets = bsd_reallocf(x->hit_offsets, hit_offset_size);
  if (!x->hit_offsets) return H3C_ENOMEM;

  x->hit_offsets[0] = hit_offset0;
  for (uint64_t i = 1; i < x->nhits; i++)
  {
    x->hit_offsets[i] = parse_uint64(head);
    head += 8;
  }

  *size = (size_t)(head - old_head);
  return 0;
}

struct alidisplay
{
  uint8_t presence;

  char *rfline;
  char *mmline;
  char *csline;
  char *model;
  char *mline;
  char *aseq;
  char *ntseq;
  char *ppline;
  uint32_t N;

  char *hmmname;
  char *hmmacc;
  char *hmmdesc;
  uint32_t hmmfrom;
  uint32_t hmmto;
  uint32_t M;

  char *sqname;
  char *sqacc;
  char *sqdesc;
  uint64_t sqfrom;
  uint64_t sqto;
  uint64_t L;

  size_t memsize;
  unsigned char *mem;
};

static void alidisplay_init(struct alidisplay *x)
{
  x->presence = 0;

  x->rfline = NULL;
  x->mmline = NULL;
  x->csline = NULL;
  x->model  = NULL;
  x->mline  = NULL;
  x->aseq   = NULL;
  x->ntseq  = NULL;
  x->ppline = NULL;
  x->N      = 0;

  x->hmmname = NULL;
  x->hmmacc  = NULL;
  x->hmmdesc = NULL;
  x->hmmfrom = 0;
  x->hmmto   = 0;
  x->M       = 0;

  x->sqname = NULL;
  x->sqacc  = NULL;
  x->sqdesc = NULL;
  x->sqfrom = 0;
  x->sqto   = 0;
  x->L      = 0;

  x->memsize = 0;
  x->mem     = NULL;
}

static void alidisplay_cleanup(struct alidisplay *x)
{
  free(x->mem);
  alidisplay_init(x);
}

#define SER_BASE_SIZE ((5 * sizeof(int)) + (3 * sizeof(int64_t)) + 1)

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT   (1 << 4)
#define NTSEQ_PRESENT  (1 << 5)

static inline bool has_string(unsigned char const *head,
                              unsigned char const *tail)
{
  char const *string = (char const *)head;
  return posix_strnlen(string, tail - head) < (size_t)(tail - head);
}

static int alidisplay_parse(struct alidisplay *x, unsigned char const *head,
                            unsigned char const *tail, size_t *size)
{
  int rc = 0;
  unsigned char const *old_head = head;

  if (tail < head + 1 + 5 * 4 + 3 * 8) defer_return(H3C_EPARSE);

  size_t obj_size = parse_uint32(head); head += 4;
  if (obj_size <= SER_BASE_SIZE) defer_return(H3C_EPARSE);
  size_t memsize = (size_t)(obj_size - SER_BASE_SIZE);

  if (!(x->mem = bsd_reallocf(x->mem, memsize))) defer_return(H3C_ENOMEM);
  x->memsize = memsize;

  x->N       = parse_uint32(head); head += 4;
  x->hmmfrom = parse_uint32(head); head += 4;
  x->hmmto   = parse_uint32(head); head += 4;
  x->M       = parse_uint32(head); head += 4;
  x->sqfrom  = parse_uint64(head); head += 8;
  x->sqto    = parse_uint64(head); head += 8;
  x->L       = parse_uint64(head); head += 8;

  x->presence = parse_flag(head); head += 1;

  if (tail < head + memsize) defer_return(H3C_EPARSE);
  memcpy(x->mem, head, memsize);
  head += memsize;

  unsigned char *mem = x->mem;
  unsigned char *memtail = mem  + memsize;

  if (x->presence & RFLINE_PRESENT && !has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->rfline = x->presence & RFLINE_PRESENT ? (char *)mem          : NULL;
  mem      += x->presence & RFLINE_PRESENT ? strlen(x->rfline) + 1: 0;

  if (x->presence & MMLINE_PRESENT && !has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->mmline = x->presence & MMLINE_PRESENT ? (char *)mem          : NULL;
  mem      += x->presence & MMLINE_PRESENT ? strlen(x->mmline) + 1: 0;

  if (x->presence & CSLINE_PRESENT && !has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->csline = x->presence & CSLINE_PRESENT ? (char *)mem          : NULL;
  mem      += x->presence & CSLINE_PRESENT ? strlen(x->csline) + 1: 0;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->model = (char *)mem; mem += strlen(x->model) + 1;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->mline = (char *)mem; mem += strlen(x->mline) + 1;

  if (x->presence & ASEQ_PRESENT && !has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->aseq = x->presence & ASEQ_PRESENT ? (char *)mem        : NULL;
  mem    += x->presence & ASEQ_PRESENT ? strlen(x->aseq) + 1: 0;

  if (x->presence & NTSEQ_PRESENT && !has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->ntseq = x->presence & NTSEQ_PRESENT ? (char *)mem         : NULL;
  mem     += x->presence & NTSEQ_PRESENT ? strlen(x->ntseq) + 1: 0;

  if (x->presence & PPLINE_PRESENT && !has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->ppline = x->presence & PPLINE_PRESENT ? (char *)mem          : NULL;
  mem      += x->presence & PPLINE_PRESENT ? strlen(x->ppline) + 1: 0;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->hmmname = (char *)mem; mem += strlen(x->hmmname) + 1;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->hmmacc = (char *)mem; mem += strlen(x->hmmacc) + 1;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->hmmdesc = (char *)mem; mem += strlen(x->hmmdesc) + 1;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->sqname = (char *)mem; mem += strlen(x->sqname) + 1;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->sqacc = (char *)mem; mem += strlen(x->sqacc) + 1;

  if (!has_string(mem, memtail)) defer_return(H3C_EPARSE);
  x->sqdesc = (char *)mem; mem += strlen(x->sqdesc) + 1;

  *size = (size_t)(head - old_head);
  return 0;

defer:
  alidisplay_cleanup(x);
  return rc;
}

struct domain
{
  uint32_t ienv;
  uint32_t jenv;
  uint32_t iali;
  uint32_t jali;
  // The following two members seems to be receiving
  // random numbers from daemon end. Skip them.
  // int64_t iorf, jorf;
  float envsc;
  float domcorrection;
  float dombias;
  float oasc;
  float bitscore;
  double lnP;
  bool is_reported;
  bool is_included;
  uint32_t scores_size;
  float *pos_score;
  struct alidisplay ad;
};

static void domain_init(struct domain *x)
{
  x->ienv = 0;
  x->jenv = 0;
  x->iali = 0;
  x->jali = 0;
  // The following two members seems to be receiving
  // random numbers from daemon end. Skip them.
  // int64_t iorf, jorf;
  x->envsc         = 0;
  x->domcorrection = 0;
  x->dombias       = 0;
  x->oasc          = 0;
  x->bitscore      = 0;
  x->lnP           = 0;
  x->is_reported   = false;
  x->is_included   = false;
  x->scores_size   = 0;
  x->pos_score     = NULL;
  alidisplay_init(&x->ad);
}

static void domain_cleanup(struct domain *x)
{
  free(x->pos_score);
  alidisplay_cleanup(&x->ad);
  domain_init(x);
}

static int domain_parse(struct domain *x, unsigned char const *head,
                        unsigned char const *tail, size_t *size)
{
  int rc = 0;
  unsigned char const *old_head = head;

  if (tail < head + 4 + 6 * 8) defer_return(H3C_EPARSE);

  // Skips object size
  (void)parse_uint32(head)    ; head += 4;
  x->ienv = parse_uint64(head); head += 8;
  x->jenv = parse_uint64(head); head += 8;
  x->iali = parse_uint64(head); head += 8;
  x->jali = parse_uint64(head); head += 8;

  // Skips iorf and jorf.
  (void)parse_uint64(head); head += 8;
  (void)parse_uint64(head); head += 8;

  if (tail < head + 8 * 4 + 8) defer_return(H3C_EPARSE);

  x->envsc         = parse_float(head) ; head += 4;
  x->domcorrection = parse_float(head) ; head += 4;
  x->dombias       = parse_float(head) ; head += 4;
  x->oasc          = parse_float(head) ; head += 4;
  x->bitscore      = parse_float(head) ; head += 4;
  x->lnP           = parse_double(head); head += 8;
  x->is_reported   = parse_uint32(head); head += 4;
  x->is_included   = parse_uint32(head); head += 4;

  uint32_t scores_size = parse_uint32(head); head += 4;

  if (scores_size > x->scores_size)
  {
    size_t size = scores_size * sizeof(float);
    if (!(x->pos_score = bsd_reallocf(x->pos_score, size)))
      defer_return(H3C_EPARSE);
    x->scores_size = scores_size;
  }
  else
    x->scores_size = scores_size;

  if (tail < head + x->scores_size * 4) defer_return(H3C_EPARSE);
  for (uint32_t i = 0; i < x->scores_size; i++)
  {
    x->pos_score[i] = parse_float(head); head += 4;
  }

  size_t n = 0;
  if ((rc = alidisplay_parse(&x->ad, head, tail, &n))) defer_return(rc);
  head += n;

  *size = (size_t)(head - old_head);
  return 0;

defer:
  domain_cleanup(x);
  return rc;
}

struct hit
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

  struct domain *dcl;
};

static void hit_init(struct hit *x)
{
  x->name = NULL;
  x->acc  = NULL;
  x->desc = NULL;
  // The following members seem to be receiving
  // random numbers from daemon end. Skip them.
  // int window_length;
  x->sortkey =  0;

  x->score     = 0;
  x->pre_score = 0;
  x->sum_score = 0;

  x->lnP     = 0;
  x->pre_lnP = 0;
  x->sum_lnP = 0;

  x->nexpected  = 0;
  x->nregions   = 0;
  x->nclustered = 0;
  x->noverlaps  = 0;
  x->nenvelopes = 0;
  x->ndom       = 0;

  x->flags       = 0;
  x->nreported   = 0;
  x->nincluded   = 0;
  x->best_domain = 0;

  x->dcl = NULL;
}

static void hit_cleanup(struct hit *x)
{
  for (uint32_t i = 0; i < x->ndom; i++)
    domain_cleanup(x->dcl + i);
  free(x->dcl);
  free(x->name);
  free(x->acc);
  free(x->desc);
  hit_init(x);
}

static int copy_string(char **dst, char const *src)
{
  if (!src) return 0;
  size_t size = strlen(src) + 1;
  if (!(*dst = bsd_reallocf(*dst, size))) return H3C_ENOMEM;
  memcpy(*dst, src, size);
  return 0;
}

#define ACC_PRESENT  (1 << 0)
#define DESC_PRESENT (1 << 1)

static int hit_parse(struct hit *x, unsigned char const *head,
                     unsigned char const *tail, size_t *size)
{
  int rc = 0;
  unsigned char const *old_head = head;

  if (tail < head + 2 * 4) defer_return(H3C_EPARSE);

  // Skips object size
  (void)parse_uint32(head); head += 4;

  // Skips window_length for now
  (void)parse_uint32(head); head += 4;

  if (tail < head + 4 * 8 + 3 * 4) defer_return(H3C_EPARSE);
  x->sortkey   = parse_double(head); head += 8;
  x->score     = parse_float(head) ; head += 4;
  x->pre_score = parse_float(head) ; head += 4;
  x->sum_score = parse_float(head) ; head += 4;

  x->lnP       = parse_double(head); head += 8;
  x->pre_lnP   = parse_double(head); head += 8;
  x->sum_lnP   = parse_double(head); head += 8;

  if (tail < head + 6 * 4) defer_return(H3C_EPARSE);
  x->nexpected  = parse_float(head) ; head += 4;
  x->nregions   = parse_uint32(head); head += 4;
  x->nclustered = parse_uint32(head); head += 4;
  x->noverlaps  = parse_uint32(head); head += 4;
  x->nenvelopes = parse_uint32(head); head += 4;
  uint32_t ndom = parse_uint32(head); head += 4;

  if (tail < head + 4 * 4 + 2 * 8) defer_return(H3C_EPARSE);
  x->flags       = parse_uint32(head); head += 4;
  x->nreported   = parse_uint32(head); head += 4;
  x->nincluded   = parse_uint32(head); head += 4;
  x->best_domain = parse_uint32(head); head += 4;
  // Skips seqidx
  (void)parse_uint64(head); head += 8;
  // Skips subseq_start for now
  (void)parse_uint64(head); head += 8;

  if (tail < head + 1) defer_return(H3C_EPARSE);
  uint8_t presence = parse_flag(head); head += 1;

  if (!has_string(head, tail)) defer_return(H3C_EPARSE);
  if ((rc = copy_string(&x->name, (char const *)head))) defer_return(rc);
  head += strlen(x->name) + 1;

  if (presence & ACC_PRESENT)
  {
    if (!has_string(head, tail)) defer_return(H3C_EPARSE);
    if ((rc = copy_string(&x->acc, (char const *)head))) defer_return(rc);
    head += strlen(x->acc) + 1;
  }

  if (presence & DESC_PRESENT)
  {
    if (!has_string(head, tail)) defer_return(H3C_EPARSE);
    if ((rc = copy_string(&x->desc, (char const *)head))) defer_return(rc);
    head += strlen(x->desc) + 1;
  }

  if (ndom > x->ndom)
  {
    if (!(x->dcl = bsd_reallocf(x->dcl, ndom * sizeof(*x->dcl)))) defer_return(H3C_ENOMEM);
    for (uint32_t i = x->ndom; i < ndom; i++)
    {
      domain_init(x->dcl + i);
      ++x->ndom;
    }
  }
  else
  {
    for (uint32_t i = ndom; i < x->ndom; i++)
      domain_cleanup(x->dcl + i);
    x->ndom = ndom;
  }

  for (uint32_t i = 0; i < x->ndom; i++)
  {
    size_t n = 0;
    if ((rc = domain_parse(x->dcl + i, head, tail, &n))) defer_return(rc);
    head += n;
  }

  *size = (size_t)(head - old_head);
  return 0;

defer:
  hit_cleanup(x);
  return rc;
}

struct tophits
{
  struct hit **hit;
  struct hit *unsrt;
  uint64_t nhits;
  uint64_t nreported;
  uint64_t nincluded;
  bool is_sorted_by_sortkey;
  bool is_sorted_by_seqidx;
};

static void tophits_init(struct tophits *x)
{
  x->hit                  = NULL;
  x->unsrt                = NULL;
  x->nhits                = 0;
  x->nreported            = 0;
  x->nincluded            = 0;
  x->is_sorted_by_sortkey = false;
  x->is_sorted_by_seqidx  = false;
}

static void tophits_cleanup(struct tophits *x)
{
  for (uint64_t i = 0; i < x->nhits; ++i)
    hit_cleanup(x->unsrt + i);
  free(x->hit);
  free(x->unsrt);
  tophits_init(x);
}

static int tophits_parse(struct tophits *x, unsigned char const *head,
                         unsigned char const *tail, size_t *size, uint64_t nhits,
                         uint64_t nreported, uint64_t nincluded)
{
  int rc = 0;
  unsigned char const *old_head = head;

  if (nhits > x->nhits)
  {
    if (!(x->hit = bsd_reallocf(x->hit, sizeof(struct hit *) * nhits)))
      defer_return(H3C_ENOMEM);

    if (!(x->unsrt = bsd_reallocf(x->unsrt, sizeof(struct hit) * nhits)))
      defer_return(H3C_ENOMEM);

    for (uint64_t i = x->nhits; i < nhits; ++i)
    {
      hit_init(x->unsrt + i);
      ++x->nhits;
    }
  }

  x->nreported            = nreported;
  x->nincluded            = nincluded;
  x->is_sorted_by_seqidx  = false;
  x->is_sorted_by_sortkey = true;

  for (uint64_t i = 0; i < nhits; ++i)
  {
    int rc = 0;
    size_t n = 0;
    if ((rc = hit_parse(x->unsrt + i, head, tail, &n))) defer_return(rc);
    head += n;
    x->hit[i] = x->unsrt + i;
  }

  *size = (size_t)(head - old_head);
  return 0;

defer:
  tophits_cleanup(x);
  return rc;
}

struct body
{
  struct stats stats;
  struct tophits tophits;
};

struct body *body_new(void)
{
  struct body *x = malloc(sizeof(struct body));
  if (!x) return NULL;
  stats_init(&x->stats);
  tophits_init(&x->tophits);
  return x;
}

int body_parse(struct body *x, size_t size,
               unsigned char const buffer[size])
{
  int rc = 0;
  size_t n = 0;
  unsigned char const *head = buffer;
  unsigned char const *tail = buffer + size;

  if ((rc = stats_parse(&x->stats, head, tail, &n))) defer_return(rc);
  head += n;

  if ((rc = tophits_parse(&x->tophits, head, tail, &n, x->stats.nhits,
                          x->stats.nreported, x->stats.nincluded)))
    defer_return(rc);
  head += n;

  if (head != tail) defer_return(H3C_EPARSE);

  return rc;

defer:
  stats_cleanup(&x->stats);
  tophits_cleanup(&x->tophits);
  return rc;
}

static int copy_stats(struct h3r_stats *dst, struct stats const *src)
{
  dst->Z    = src->Z;
  dst->domZ = src->domZ;

  dst->Z_setby    = (enum h3r_zsetby)src->Z_setby;
  dst->domZ_setby = (enum h3r_zsetby)src->domZ_setby;

  if (src->nmodels     > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->nseqs       > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->n_past_msv  > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->n_past_bias > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->n_past_vit  > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->n_past_fwd  > UINT32_MAX) return H3C_EOUTRANGE;

  dst->nmodels     = src->nmodels;
  dst->nseqs       = src->nseqs;
  dst->n_past_msv  = src->n_past_msv;
  dst->n_past_bias = src->n_past_bias;
  dst->n_past_vit  = src->n_past_vit;
  dst->n_past_fwd  = src->n_past_fwd;

  if (src->nhits     > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->nreported > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->nincluded > UINT32_MAX) return H3C_EOUTRANGE;

  dst->nhits     = src->nhits;
  dst->nreported = src->nreported;
  dst->nincluded = src->nincluded;

  return 0;
}

static int copy_alidisplay(struct h3r_alidisplay *dst,
                           struct alidisplay const *src)
{
  dst->presence = src->presence;

  if (copy_string(&dst->rfline, src->rfline)) return H3C_ENOMEM;
  if (copy_string(&dst->mmline, src->mmline)) return H3C_ENOMEM;
  if (copy_string(&dst->csline, src->csline)) return H3C_ENOMEM;
  if (copy_string(&dst->model, src->model))   return H3C_ENOMEM;
  if (copy_string(&dst->mline, src->mline))   return H3C_ENOMEM;
  if (copy_string(&dst->aseq, src->aseq))     return H3C_ENOMEM;
  if (copy_string(&dst->ntseq, src->ntseq))   return H3C_ENOMEM;
  if (copy_string(&dst->ppline, src->ppline)) return H3C_ENOMEM;
  dst->N = src->N;

  if (copy_string(&dst->hmmname, src->hmmname)) return H3C_ENOMEM;
  if (copy_string(&dst->hmmacc, src->hmmacc))   return H3C_ENOMEM;
  if (copy_string(&dst->hmmdesc, src->hmmdesc)) return H3C_ENOMEM;
  dst->hmmfrom = src->hmmfrom;
  dst->hmmto = src->hmmto;
  dst->M = src->M;

  if (src->sqfrom > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->sqto   > UINT32_MAX) return H3C_EOUTRANGE;
  if (src->L      > UINT32_MAX) return H3C_EOUTRANGE;

  if (copy_string(&dst->sqname, src->sqname)) return H3C_ENOMEM;
  if (copy_string(&dst->sqacc, src->sqacc))   return H3C_ENOMEM;
  if (copy_string(&dst->sqdesc, src->sqdesc)) return H3C_ENOMEM;
  dst->sqfrom = src->sqfrom;
  dst->sqto = src->sqto;
  dst->L = src->L;

  return 0;
}

static int copy_domain(struct h3r_domain *dst, struct domain const *src)
{
  int rc = 0;
  if ((rc = h3r_domain_setup(dst, src->scores_size))) defer_return(H3C_EH3RESULT);

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
  memcpy(dst->pos_score, src->pos_score, dst->pos_score_size * 4);

  if ((rc = copy_alidisplay(&dst->ad, &src->ad))) defer_return(rc);

  return 0;

defer:
  h3r_domain_cleanup(dst);
  return rc;
}

static int copy_hit(struct h3r_hit *dst, struct hit const *src)
{
  int rc = 0;
  if (h3r_hit_setup(dst, src->ndom)) defer_return(H3C_EH3RESULT);

  if (copy_string(&dst->name, src->name)) defer_return(H3C_ENOMEM);
  if (copy_string(&dst->acc, src->acc))   defer_return(H3C_ENOMEM);
  if (copy_string(&dst->desc, src->desc)) defer_return(H3C_ENOMEM);

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

static int copy_tophits(struct h3r_tophits *dst, struct tophits const *src)
{
  int rc = 0;
  if (h3r_tophits_setup(dst, src->nhits)) defer_return(H3C_EH3RESULT);

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

int body_store(struct body *x, struct h3r *r)
{
  int rc = 0;

  if ((rc = copy_stats(&r->stats, &x->stats))) return rc;
  if ((rc = copy_tophits(&r->tophits, &x->tophits))) return rc;

  return rc;
}

void body_del(struct body const *x)
{
  if (x)
  {
    stats_cleanup((struct stats *)&x->stats);
    tophits_cleanup((struct tophits *)&x->tophits);
  }
  free((void *)x);
}
