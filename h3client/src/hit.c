#include "hit.h"
#include "domain.h"
#include "h3client/errno.h"
#include "lip/lip.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

int h3client_hit_init(struct hit *hit)
{
  memset(hit, 0, sizeof(*hit));

  if (!(hit->name = calloc(1, sizeof(char)))) goto cleanup;
  if (!(hit->acc = calloc(1, sizeof(char)))) goto cleanup;
  if (!(hit->desc = calloc(1, sizeof(char)))) goto cleanup;

  return 0;

cleanup:
  h3client_hit_cleanup(hit);
  return H3CLIENT_ENOMEM;
}

static int grow(struct hit *hit, unsigned ndomains)
{
  size_t sz = ndomains * sizeof(*hit->domains);
  struct domain *domains = realloc(hit->domains, sz);
  if (!domains) goto cleanup;
  hit->domains = domains;

  for (unsigned i = hit->ndomains; i < ndomains; ++i)
  {
    h3client_domain_init(hit->domains + i);
    ++hit->ndomains;
  }

  return 0;

cleanup:
  h3client_hit_cleanup(hit);
  return H3CLIENT_ENOMEM;
}

static void shrink(struct hit *hit, unsigned ndomains)
{
  for (unsigned i = ndomains; i < hit->ndomains; ++i)
    h3client_domain_cleanup(hit->domains + i);

  hit->ndomains = ndomains;
}

int h3client_hit_setup(struct hit *hit, unsigned ndomains)
{
  if (hit->ndomains < ndomains) return grow(hit, ndomains);
  shrink(hit, ndomains);
  return 0;
}

void h3client_hit_cleanup(struct hit *hit)
{
  DEL(hit->name);
  DEL(hit->acc);
  DEL(hit->desc);

  for (unsigned i = 0; i < hit->ndomains; ++i)
    h3client_domain_cleanup(hit->domains + i);

  hit->ndomains = 0;
  DEL(hit->domains);
}

int h3client_hit_pack(struct hit const *hit, struct lip_file *f)
{
  lip_write_array_size(f, 20);

  lip_write_cstr(f, hit->name);
  lip_write_cstr(f, hit->acc);
  lip_write_cstr(f, hit->desc);
  lip_write_float(f, hit->sortkey);

  lip_write_float(f, hit->score);
  lip_write_float(f, hit->pre_score);
  lip_write_float(f, hit->sum_score);

  lip_write_float(f, hit->lnP);
  lip_write_float(f, hit->pre_lnP);
  lip_write_float(f, hit->sum_lnP);

  lip_write_float(f, hit->nexpected);
  lip_write_int(f, hit->nregions);
  lip_write_int(f, hit->nclustered);
  lip_write_int(f, hit->noverlaps);
  lip_write_int(f, hit->nenvelopes);

  lip_write_int(f, hit->flags);
  lip_write_int(f, hit->nreported);
  lip_write_int(f, hit->nincluded);
  lip_write_int(f, hit->best_domain);

  lip_write_map_size(f, 1);
  lip_write_cstr(f, "domains");
  lip_write_array_size(f, hit->ndomains);

  for (unsigned i = 0; i < hit->ndomains; ++i)
  {
    int rc = h3client_domain_pack(hit->domains + i, f);
    if (rc) return rc;
  }

  return lip_file_error(f) ? H3CLIENT_EPACK : 0;
}

int h3client_hit_unpack(struct hit *hit, struct lip_file *f)
{
  int rc = H3CLIENT_EUNPACK;

  if (!h3client_expect_array_size(f, 20)) goto cleanup;

  if ((rc = h3client_read_string(f, &hit->name))) goto cleanup;
  if ((rc = h3client_read_string(f, &hit->acc))) goto cleanup;
  if ((rc = h3client_read_string(f, &hit->desc))) goto cleanup;

  lip_read_float(f, &hit->sortkey);

  lip_read_float(f, &hit->score);
  lip_read_float(f, &hit->pre_score);
  lip_read_float(f, &hit->sum_score);

  lip_read_float(f, &hit->lnP);
  lip_read_float(f, &hit->pre_lnP);
  lip_read_float(f, &hit->sum_lnP);

  lip_read_float(f, &hit->nexpected);
  lip_read_int(f, &hit->nregions);
  lip_read_int(f, &hit->nclustered);
  lip_read_int(f, &hit->noverlaps);
  lip_read_int(f, &hit->nenvelopes);

  lip_read_int(f, &hit->flags);
  lip_read_int(f, &hit->nreported);
  lip_read_int(f, &hit->nincluded);
  lip_read_int(f, &hit->best_domain);

  if (!h3client_expect_map_size(f, 1)) goto cleanup;
  if (!h3client_expect_key(f, "domains")) goto cleanup;

  unsigned size = 0;
  if (!lip_read_array_size(f, &size)) goto cleanup;
  if ((rc = h3client_hit_setup(hit, size))) goto cleanup;

  for (unsigned i = 0; i < hit->ndomains; ++i)
  {
    if ((rc = h3client_domain_unpack(hit->domains + i, f))) goto cleanup;
  }

  return 0;

cleanup:
  h3client_hit_cleanup(hit);
  return rc;
}
