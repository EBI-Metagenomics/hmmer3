#include "h3r_hit.h"
#include "defer_return.h"
#include "expect.h"
#include "h3r_domain.h"
#include "lio.h"
#include "rc.h"
#include "read.h"
#include "static_assert2.h"
#include "write.h"
#include <stdlib.h>
#include <string.h>

static_assert2(sizeof(unsigned) >= 4);

static inline void unset(struct h3r_hit *x)
{
  x->name        = NULL;
  x->acc         = NULL;
  x->desc        = NULL;
  x->sortkey     = 0;

  x->score       = 0;
  x->pre_score   = 0;
  x->sum_score   = 0;

  x->lnP         = 0;
  x->pre_lnP     = 0;
  x->sum_lnP     = 0;

  x->nexpected   = 0;
  x->nregions    = 0;
  x->nclustered  = 0;
  x->noverlaps   = 0;
  x->nenvelopes  = 0;

  x->flags       = 0;
  x->nreported   = 0;
  x->nincluded   = 0;
  x->best_domain = 0;

  x->ndomains    = 0;
  x->domains     = NULL;
}

int h3r_hit_init(struct h3r_hit *x)
{
  int rc = 0;
  unset(x);
  if (!(x->name = malloc(sizeof(char)))) defer_return(H3R_ENOMEM);
  if (!(x->acc = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);
  if (!(x->desc = malloc(sizeof(char)))) defer_return(H3R_ENOMEM);

  x->name[0] = '\0';
  x->acc[0]  = '\0';
  x->desc[0] = '\0';

  return 0;

defer:
  h3r_hit_cleanup(x);
  return rc;
}

static int grow(struct h3r_hit *x, unsigned ndomains)
{
  int rc = 0;

  size_t sz = ndomains * sizeof(*x->domains);
  x->domains = realloc(x->domains, sz);
  if (!x->domains) defer_return(H3R_ENOMEM);

  for (unsigned i = x->ndomains; i < ndomains; ++i)
  {
    if ((rc = h3r_domain_init(x->domains + i))) return rc;
    ++x->ndomains;
  }

  return 0;

defer:
  return rc;
}

static void shrink(struct h3r_hit *x, unsigned ndomains)
{
  for (unsigned i = ndomains; i < x->ndomains; ++i)
    h3r_domain_cleanup(x->domains + i);

  x->ndomains = ndomains;
}

int h3r_hit_setup(struct h3r_hit *x, unsigned ndomains)
{
  if (x->ndomains < ndomains) return grow(x, ndomains);
  shrink(x, ndomains);
  return 0;
}

void h3r_hit_cleanup(struct h3r_hit *x)
{
  free(x->name);
  free(x->acc);
  free(x->desc);

  for (unsigned i = 0; i < x->ndomains; ++i)
    h3r_domain_cleanup(x->domains + i);

  free(x->domains);
  unset(x);
}

int h3r_hit_pack(struct h3r_hit const *x, struct lio_writer *f)
{
  if (write_array(f, 20))           return H3R_EPACK;

  if (write_cstring(f, x->name))    return H3R_EPACK;
  if (write_cstring(f, x->acc))     return H3R_EPACK;
  if (write_cstring(f, x->desc))    return H3R_EPACK;
  if (write_float(f, x->sortkey))   return H3R_EPACK;

  if (write_float(f, x->score))     return H3R_EPACK;
  if (write_float(f, x->pre_score)) return H3R_EPACK;
  if (write_float(f, x->sum_score)) return H3R_EPACK;

  if (write_float(f, x->lnP))       return H3R_EPACK;
  if (write_float(f, x->pre_lnP))   return H3R_EPACK;
  if (write_float(f, x->sum_lnP))   return H3R_EPACK;

  if (write_float(f, x->nexpected)) return H3R_EPACK;
  if (write_int(f, x->nregions))    return H3R_EPACK;
  if (write_int(f, x->nclustered))  return H3R_EPACK;
  if (write_int(f, x->noverlaps))   return H3R_EPACK;
  if (write_int(f, x->nenvelopes))  return H3R_EPACK;

  if (write_int(f, x->flags))       return H3R_EPACK;
  if (write_int(f, x->nreported))   return H3R_EPACK;
  if (write_int(f, x->nincluded))   return H3R_EPACK;
  if (write_int(f, x->best_domain)) return H3R_EPACK;

  if (write_map(f, 1))              return H3R_EPACK;
  if (write_cstring(f, "domains"))  return H3R_EPACK;
  if (write_array(f, x->ndomains))  return H3R_EPACK;

  for (unsigned i = 0; i < x->ndomains; ++i)
  {
    int rc = h3r_domain_pack(x->domains + i, f);
    if (rc) return rc;
  }

  return 0;
}

int h3r_hit_unpack(struct h3r_hit *x, struct lio_reader *f)
{
  int rc = 0;

  if (expect_array(f, 20))               return H3R_EUNPACK;

  if ((rc = read_cstring2(f, &x->name))) return H3R_EUNPACK;
  if ((rc = read_cstring2(f, &x->acc)))  return H3R_EUNPACK;
  if ((rc = read_cstring2(f, &x->desc))) return H3R_EUNPACK;

  if (read_float(f, &x->sortkey))        return H3R_EUNPACK;

  if (read_float(f, &x->score))          return H3R_EUNPACK;
  if (read_float(f, &x->pre_score))      return H3R_EUNPACK;
  if (read_float(f, &x->sum_score))      return H3R_EUNPACK;

  if (read_float(f, &x->lnP))            return H3R_EUNPACK;
  if (read_float(f, &x->pre_lnP))        return H3R_EUNPACK;
  if (read_float(f, &x->sum_lnP))        return H3R_EUNPACK;

  if (read_float(f, &x->nexpected))      return H3R_EUNPACK;
  if (read_int(f, &x->nregions))         return H3R_EUNPACK;
  if (read_int(f, &x->nclustered))       return H3R_EUNPACK;
  if (read_int(f, &x->noverlaps))        return H3R_EUNPACK;
  if (read_int(f, &x->nenvelopes))       return H3R_EUNPACK;

  if (read_int(f, &x->flags))            return H3R_EUNPACK;
  if (read_int(f, &x->nreported))        return H3R_EUNPACK;
  if (read_int(f, &x->nincluded))        return H3R_EUNPACK;
  if (read_int(f, &x->best_domain))      return H3R_EUNPACK;

  if (expect_map(f, 1))                  return H3R_EUNPACK;
  if (expect_key(f, "domains"))          return H3R_EUNPACK;

  unsigned size = 0;
  if (read_array(f, &size))              return H3R_EUNPACK;

  if ((rc = h3r_hit_setup(x, size))) return rc;

  for (unsigned i = 0; i < x->ndomains; ++i)
  {
    if ((rc = h3r_domain_unpack(x->domains + i, f))) return rc;
  }

  return 0;
}
