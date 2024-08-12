#include "hit.h"
#include "defer_return.h"
#include "domain.h"
#include "expect.h"
#include "lio.h"
#include "rc.h"
#include "read.h"
#include "write.h"
#include <stdlib.h>
#include <string.h>

static inline void unset(struct hit *x)
{
  x->name = NULL;
  x->acc = NULL;
  x->desc = NULL;
  x->sortkey = 0;

  x->score = 0;
  x->pre_score = 0;
  x->sum_score = 0;

  x->lnP = 0;
  x->pre_lnP = 0;
  x->sum_lnP = 0;

  x->nexpected = 0;
  x->nregions = 0;
  x->nclustered = 0;
  x->noverlaps = 0;
  x->nenvelopes = 0;

  x->flags = 0;
  x->nreported = 0;
  x->nincluded = 0;
  x->best_domain = 0;

  x->ndomains = 0;
  x->domains = NULL;
}

int h3result_hit_init(struct hit *x)
{
  int rc = 0;
  unset(x);
  if (!(x->name = malloc(sizeof(char)))) defer_return(H3RESULT_ENOMEM);
  if (!(x->acc = malloc(sizeof(char)))) defer_return(H3RESULT_ENOMEM);
  if (!(x->desc = malloc(sizeof(char)))) defer_return(H3RESULT_ENOMEM);

  return 0;

defer:
  h3result_hit_cleanup(x);
  return rc;
}

static int grow(struct hit *hit, unsigned ndomains)
{
  int rc = 0;

  size_t sz = ndomains * sizeof(*hit->domains);
  hit->domains = realloc(hit->domains, sz);
  if (!hit->domains) defer_return(H3RESULT_ENOMEM);

  for (unsigned i = hit->ndomains; i < ndomains; ++i)
  {
    if ((rc = h3result_domain_init(hit->domains + i))) return rc;
    ++hit->ndomains;
  }

  return 0;

defer:
  return rc;
}

static void shrink(struct hit *hit, unsigned ndomains)
{
  for (unsigned i = ndomains; i < hit->ndomains; ++i)
    h3result_domain_cleanup(hit->domains + i);

  hit->ndomains = ndomains;
}

int h3result_hit_setup(struct hit *hit, unsigned ndomains)
{
  if (hit->ndomains < ndomains) return grow(hit, ndomains);
  shrink(hit, ndomains);
  return 0;
}

void h3result_hit_cleanup(struct hit *hit)
{
  free(hit->name);
  free(hit->acc);
  free(hit->desc);

  hit->name = NULL;
  hit->acc = NULL;
  hit->desc = NULL;

  for (unsigned i = 0; i < hit->ndomains; ++i)
    h3result_domain_cleanup(hit->domains + i);

  hit->ndomains = 0;
  free(hit->domains);
  hit->domains = NULL;
}

int h3result_hit_pack(struct hit const *hit, struct lio_writer *f)
{
  if (write_array(f, 20)) return H3RESULT_EPACK;

  if (write_cstring(f, hit->name)) return H3RESULT_EPACK;
  if (write_cstring(f, hit->acc)) return H3RESULT_EPACK;
  if (write_cstring(f, hit->desc)) return H3RESULT_EPACK;
  if (write_float(f, hit->sortkey)) return H3RESULT_EPACK;

  if (write_float(f, hit->score)) return H3RESULT_EPACK;
  if (write_float(f, hit->pre_score)) return H3RESULT_EPACK;
  if (write_float(f, hit->sum_score)) return H3RESULT_EPACK;

  if (write_float(f, hit->lnP)) return H3RESULT_EPACK;
  if (write_float(f, hit->pre_lnP)) return H3RESULT_EPACK;
  if (write_float(f, hit->sum_lnP)) return H3RESULT_EPACK;

  if (write_float(f, hit->nexpected)) return H3RESULT_EPACK;
  if (write_int(f, hit->nregions)) return H3RESULT_EPACK;
  if (write_int(f, hit->nclustered)) return H3RESULT_EPACK;
  if (write_int(f, hit->noverlaps)) return H3RESULT_EPACK;
  if (write_int(f, hit->nenvelopes)) return H3RESULT_EPACK;

  if (write_int(f, hit->flags)) return H3RESULT_EPACK;
  if (write_int(f, hit->nreported)) return H3RESULT_EPACK;
  if (write_int(f, hit->nincluded)) return H3RESULT_EPACK;
  if (write_int(f, hit->best_domain)) return H3RESULT_EPACK;

  if (write_map(f, 1)) return H3RESULT_EPACK;
  if (write_cstring(f, "domains")) return H3RESULT_EPACK;
  if (write_array(f, hit->ndomains)) return H3RESULT_EPACK;

  for (unsigned i = 0; i < hit->ndomains; ++i)
  {
    int rc = h3result_domain_pack(hit->domains + i, f);
    if (rc) return rc;
  }

  return 0;
}

int h3result_hit_unpack(struct hit *hit, struct lio_reader *f)
{
  int rc = 0;

  if (!expect_array(f, 20)) return H3RESULT_EUNPACK;

  if ((rc = read_cstring2(f, &hit->name))) return H3RESULT_EUNPACK;
  if ((rc = read_cstring2(f, &hit->acc))) return H3RESULT_EUNPACK;
  if ((rc = read_cstring2(f, &hit->desc))) return H3RESULT_EUNPACK;

  if (read_float(f, &hit->sortkey)) return H3RESULT_EUNPACK;

  if (read_float(f, &hit->score)) return H3RESULT_EUNPACK;
  if (read_float(f, &hit->pre_score)) return H3RESULT_EUNPACK;
  if (read_float(f, &hit->sum_score)) return H3RESULT_EUNPACK;

  if (read_float(f, &hit->lnP)) return H3RESULT_EUNPACK;
  if (read_float(f, &hit->pre_lnP)) return H3RESULT_EUNPACK;
  if (read_float(f, &hit->sum_lnP)) return H3RESULT_EUNPACK;

  if (read_float(f, &hit->nexpected)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->nregions)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->nclustered)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->noverlaps)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->nenvelopes)) return H3RESULT_EUNPACK;

  if (read_int(f, &hit->flags)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->nreported)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->nincluded)) return H3RESULT_EUNPACK;
  if (read_int(f, &hit->best_domain)) return H3RESULT_EUNPACK;

  if (!expect_map(f, 1)) return H3RESULT_EUNPACK;
  if (!expect_key(f, "domains")) return H3RESULT_EUNPACK;

  unsigned size = 0;
  if (read_array(f, &size)) return H3RESULT_EUNPACK;

  if ((rc = h3result_hit_setup(hit, size))) return rc;

  for (unsigned i = 0; i < hit->ndomains; ++i)
  {
    if ((rc = h3result_domain_unpack(hit->domains + i, f))) return rc;
  }

  return 0;
}
