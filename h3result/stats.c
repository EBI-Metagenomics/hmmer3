#include "stats.h"
#include "expect.h"
#include "rc.h"
#include "read.h"
#include "stats.h"
#include "write.h"
#include <string.h>

void h3result_stats_init(struct stats *stats)
{
  memset(stats, 0, sizeof(*stats));
}

int h3result_stats_pack(struct stats const *stats, struct lio_writer *f)
{
  if (write_array(f, 13)) return H3RESULT_EPACK;

  if (write_float(f, stats->Z)) return H3RESULT_EPACK;
  if (write_float(f, stats->domZ)) return H3RESULT_EPACK;

  if (write_int(f, stats->Z_setby)) return H3RESULT_EPACK;
  if (write_int(f, stats->domZ_setby)) return H3RESULT_EPACK;

  if (write_int(f, stats->nmodels)) return H3RESULT_EPACK;
  if (write_int(f, stats->nseqs)) return H3RESULT_EPACK;
  if (write_int(f, stats->n_past_msv)) return H3RESULT_EPACK;
  if (write_int(f, stats->n_past_bias)) return H3RESULT_EPACK;
  if (write_int(f, stats->n_past_vit)) return H3RESULT_EPACK;
  if (write_int(f, stats->n_past_fwd)) return H3RESULT_EPACK;

  if (write_int(f, stats->nhits)) return H3RESULT_EPACK;
  if (write_int(f, stats->nreported)) return H3RESULT_EPACK;
  if (write_int(f, stats->nincluded)) return H3RESULT_EPACK;

  return 0;
}

int h3result_stats_unpack(struct stats *stats, struct lio_reader *f)
{
  if (!expect_array(f, 13)) return H3RESULT_EUNPACK;

  if (read_float(f, &stats->Z)) return H3RESULT_EUNPACK;
  if (read_float(f, &stats->domZ)) return H3RESULT_EUNPACK;

  if (read_int(f, &stats->Z_setby)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->domZ_setby)) return H3RESULT_EUNPACK;

  if (read_int(f, &stats->nmodels)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->nseqs)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->n_past_msv)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->n_past_bias)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->n_past_vit)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->n_past_fwd)) return H3RESULT_EUNPACK;

  if (read_int(f, &stats->nhits)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->nreported)) return H3RESULT_EUNPACK;
  if (read_int(f, &stats->nincluded)) return H3RESULT_EUNPACK;

  return 0;
}
