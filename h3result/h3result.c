#include "h3result.h"
#include "expect.h"
#include "fdup.h"
#include "h3result_struct.h"
#include "lio.h"
#include "rc.h"
#include "write.h"
#include <stdlib.h>
#include <unistd.h>

struct h3result *h3result_new(void)
{
  struct h3result *x = malloc(sizeof(*x));
  if (!x) return 0;
  x->errnum = 0;
  x->errstr = NULL;
  h3result_stats_init(&x->stats);
  h3result_tophits_init(&x->tophits);
  return x;
}

void h3result_del(struct h3result const *result)
{
  if (result->errstr) free((void *)result->errstr);
  h3result_tophits_cleanup((struct tophits *)&result->tophits);
  free((void *)result);
}

int h3result_pack(struct h3result const *result, int fd)
{
  struct lio_writer f = {0};
  lio_wsetup(&f, fd);

  if (write_map(&f, 1)) return H3RESULT_EPACK;
  if (write_cstring(&f, "h3result")) return H3RESULT_EPACK;

  if (write_map(&f, 2)) return H3RESULT_EPACK;
  if (write_cstring(&f, "stats")) return H3RESULT_EPACK;
  int rc = h3result_stats_pack(&result->stats, &f);
  if (rc) return rc;

  if (write_cstring(&f, "tophits")) return H3RESULT_EPACK;
  return h3result_tophits_pack(&result->tophits, &f);
}

int h3result_unpack(struct h3result *result, int fd)
{
  struct lio_reader f = {0};
  lio_rsetup(&f, fd);

  if (!expect_map(&f, 1)) return H3RESULT_EUNPACK;

  if (!expect_key(&f, "h3result")) return H3RESULT_EUNPACK;

  if (!expect_map(&f, 2)) return H3RESULT_EUNPACK;
  if (!expect_key(&f, "stats")) return H3RESULT_EUNPACK;
  int rc = h3result_stats_unpack(&result->stats, &f);
  if (rc) return rc;

  if (!expect_key(&f, "tophits")) return H3RESULT_EUNPACK;
  return h3result_tophits_unpack(&result->tophits, &f);
}

int h3result_errnum(struct h3result const *x) { return x->errnum; }

char const *h3result_errstr(struct h3result const *x) { return x->errstr; }

int h3result_print_targets(struct h3result const *r, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3RESULT_EPRINT;
  h3result_tophits_print_targets(&r->tophits, fp, r->stats.Z);
  return fclose(fp) ? H3RESULT_EPRINT : 0;
}

int h3result_print_domains(struct h3result const *r, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3RESULT_EPRINT;
  h3result_tophits_print_domains(&r->tophits, fp, r->stats.Z, r->stats.domZ);
  return fclose(fp) ? H3RESULT_EPRINT : 0;
}

int h3result_print_targets_table(struct h3result const *r, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3RESULT_EPRINT;
  h3result_tophits_print_targets_table("-", &r->tophits, fp, true, r->stats.Z);
  return fclose(fp) ? H3RESULT_EPRINT : 0;
}

int h3result_print_domains_table(struct h3result const *r, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3RESULT_EPRINT;
  h3result_tophits_print_domains_table("-", &r->tophits, fp, true, r->stats.Z,
                                       r->stats.domZ);
  return fclose(fp) ? H3RESULT_EPRINT : 0;
}

unsigned h3result_nhits(struct h3result const *r) { return r->tophits.nhits; }

char const *h3result_hit_name(struct h3result const *r, unsigned idx)
{
  return h3result_tophits_hit_name(&r->tophits, idx);
}

char const *h3result_hit_acc(struct h3result const *r, unsigned idx)
{
  return h3result_tophits_hit_acc(&r->tophits, idx);
}

double h3result_hit_evalue_ln(struct h3result const *r, unsigned idx)
{
  return h3result_tophits_hit_evalue_ln(&r->tophits, idx, r->stats.Z);
}
