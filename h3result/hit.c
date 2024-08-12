#include "hit.h"
#include "domain.h"
#include "expect.h"
#include "lio.h"
#include "rc.h"
#include "read.h"
#include "write.h"
#include <stdlib.h>
#include <string.h>

int h3result_hit_init(struct hit *hit)
{
  memset(hit, 0, sizeof(*hit));

  if (!(hit->name = calloc(1, sizeof(char)))) goto cleanup;
  if (!(hit->acc = calloc(1, sizeof(char)))) goto cleanup;
  if (!(hit->desc = calloc(1, sizeof(char)))) goto cleanup;

  return 0;

cleanup:
  h3result_hit_cleanup(hit);
  return H3RESULT_ENOMEM;
}

static int grow(struct hit *hit, unsigned ndomains)
{
  size_t sz = ndomains * sizeof(*hit->domains);
  struct domain *domains = realloc(hit->domains, sz);
  if (!domains) goto cleanup;
  hit->domains = domains;

  for (unsigned i = hit->ndomains; i < ndomains; ++i)
  {
    h3result_domain_init(hit->domains + i);
    ++hit->ndomains;
  }

  return 0;

cleanup:
  h3result_hit_cleanup(hit);
  return H3RESULT_ENOMEM;
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

#define DEL(ptr)                                                               \
  do                                                                           \
  {                                                                            \
    free((ptr));                                                               \
    (ptr) = NULL;                                                              \
  } while (0);

void h3result_hit_cleanup(struct hit *hit)
{
  DEL(hit->name);
  DEL(hit->acc);
  DEL(hit->desc);

  for (unsigned i = 0; i < hit->ndomains; ++i)
    h3result_domain_cleanup(hit->domains + i);

  hit->ndomains = 0;
  DEL(hit->domains);
}

int h3result_hit_pack(struct hit const *hit, struct lio_writer *f)
{
  write_array(f, 20);

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
  int rc = H3RESULT_EUNPACK;

  if (!h3result_expect_array_size(f, 20)) goto cleanup;

  if ((rc = h3result_read_string(f, &hit->name))) goto cleanup;
  if ((rc = h3result_read_string(f, &hit->acc))) goto cleanup;
  if ((rc = h3result_read_string(f, &hit->desc))) goto cleanup;

  if (read_float(f, &hit->sortkey)) goto cleanup;

  if (read_float(f, &hit->score)) goto cleanup;
  if (read_float(f, &hit->pre_score)) goto cleanup;
  if (read_float(f, &hit->sum_score)) goto cleanup;

  if (read_float(f, &hit->lnP)) goto cleanup;
  if (read_float(f, &hit->pre_lnP)) goto cleanup;
  if (read_float(f, &hit->sum_lnP)) goto cleanup;

  if (read_float(f, &hit->nexpected)) goto cleanup;
  if (read_int(f, &hit->nregions)) goto cleanup;
  if (read_int(f, &hit->nclustered)) goto cleanup;
  if (read_int(f, &hit->noverlaps)) goto cleanup;
  if (read_int(f, &hit->nenvelopes)) goto cleanup;

  if (read_int(f, &hit->flags)) goto cleanup;
  if (read_int(f, &hit->nreported)) goto cleanup;
  if (read_int(f, &hit->nincluded)) goto cleanup;
  if (read_int(f, &hit->best_domain)) goto cleanup;

  if (!h3result_expect_map_size(f, 1)) goto cleanup;
  if (!h3result_expect_key(f, "domains")) goto cleanup;

  unsigned size = 0;
  if (read_array(f, &size)) goto cleanup;
  if ((rc = h3result_hit_setup(hit, size))) goto cleanup;

  for (unsigned i = 0; i < hit->ndomains; ++i)
  {
    if ((rc = h3result_domain_unpack(hit->domains + i, f))) goto cleanup;
  }

  return 0;

cleanup:
  h3result_hit_cleanup(hit);
  return rc;
}
