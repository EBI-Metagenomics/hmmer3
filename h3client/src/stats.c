#include "stats.h"
#include "h3client/errno.h"
#include "lip/lip.h"
#include "stats.h"
#include "utils.h"
#include <string.h>

void h3client_stats_init(struct stats *stats)
{
  memset(stats, 0, sizeof(*stats));
}

int h3client_stats_pack(struct stats const *stats, struct lip_file *f)
{
  lip_write_array_size(f, 13);

  lip_write_float(f, stats->Z);
  lip_write_float(f, stats->domZ);

  lip_write_int(f, stats->Z_setby);
  lip_write_int(f, stats->domZ_setby);

  lip_write_int(f, stats->nmodels);
  lip_write_int(f, stats->nseqs);
  lip_write_int(f, stats->n_past_msv);
  lip_write_int(f, stats->n_past_bias);
  lip_write_int(f, stats->n_past_vit);
  lip_write_int(f, stats->n_past_fwd);

  lip_write_int(f, stats->nhits);
  lip_write_int(f, stats->nreported);
  lip_write_int(f, stats->nincluded);

  return lip_file_error(f) ? H3CLIENT_EPACK : 0;
}

int h3client_stats_unpack(struct stats *stats, struct lip_file *f)
{
  if (!h3client_expect_array_size(f, 13)) return H3CLIENT_EUNPACK;

  lip_read_float(f, &stats->Z);
  lip_read_float(f, &stats->domZ);

  lip_read_int(f, &stats->Z_setby);
  lip_read_int(f, &stats->domZ_setby);

  lip_read_int(f, &stats->nmodels);
  lip_read_int(f, &stats->nseqs);
  lip_read_int(f, &stats->n_past_msv);
  lip_read_int(f, &stats->n_past_bias);
  lip_read_int(f, &stats->n_past_vit);
  lip_read_int(f, &stats->n_past_fwd);

  lip_read_int(f, &stats->nhits);
  lip_read_int(f, &stats->nreported);
  lip_read_int(f, &stats->nincluded);

  return lip_file_error(f) ? H3CLIENT_EPACK : 0;
}
