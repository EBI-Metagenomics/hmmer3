#include "h3r_stats.h"
#include "expect.h"
#include "rc.h"
#include "read.h"
#include "write.h"

void h3r_stats_init(struct stats *x)
{
  x->Z           = 0;
  x->domZ        = 0;

  x->Z_setby     = 0;
  x->domZ_setby  = 0;

  x->nmodels     = 0;
  x->nseqs       = 0;
  x->n_past_msv  = 0;
  x->n_past_bias = 0;
  x->n_past_vit  = 0;
  x->n_past_fwd  = 0;

  x->nhits       = 0;
  x->nreported   = 0;
  x->nincluded   = 0;
}

int h3r_stats_pack(struct stats const *x, struct lio_writer *f)
{
  if (write_array(f, 13))           return H3R_EPACK;

  if (write_float(f, x->Z))         return H3R_EPACK;
  if (write_float(f, x->domZ))      return H3R_EPACK;

  if (write_int(f, x->Z_setby))     return H3R_EPACK;
  if (write_int(f, x->domZ_setby))  return H3R_EPACK;

  if (write_int(f, x->nmodels))     return H3R_EPACK;
  if (write_int(f, x->nseqs))       return H3R_EPACK;
  if (write_int(f, x->n_past_msv))  return H3R_EPACK;
  if (write_int(f, x->n_past_bias)) return H3R_EPACK;
  if (write_int(f, x->n_past_vit))  return H3R_EPACK;
  if (write_int(f, x->n_past_fwd))  return H3R_EPACK;

  if (write_int(f, x->nhits))       return H3R_EPACK;
  if (write_int(f, x->nreported))   return H3R_EPACK;
  if (write_int(f, x->nincluded))   return H3R_EPACK;

  return 0;
}

int h3r_stats_unpack(struct stats *x, struct lio_reader *f)
{
  if (expect_array(f, 13))          return H3R_EUNPACK;

  if (read_float(f, &x->Z))         return H3R_EUNPACK;
  if (read_float(f, &x->domZ))      return H3R_EUNPACK;

  if (read_int(f, &x->Z_setby))     return H3R_EUNPACK;
  if (read_int(f, &x->domZ_setby))  return H3R_EUNPACK;

  if (read_int(f, &x->nmodels))     return H3R_EUNPACK;
  if (read_int(f, &x->nseqs))       return H3R_EUNPACK;
  if (read_int(f, &x->n_past_msv))  return H3R_EUNPACK;
  if (read_int(f, &x->n_past_bias)) return H3R_EUNPACK;
  if (read_int(f, &x->n_past_vit))  return H3R_EUNPACK;
  if (read_int(f, &x->n_past_fwd))  return H3R_EUNPACK;

  if (read_int(f, &x->nhits))       return H3R_EUNPACK;
  if (read_int(f, &x->nreported))   return H3R_EUNPACK;
  if (read_int(f, &x->nincluded))   return H3R_EUNPACK;

  return 0;
}
