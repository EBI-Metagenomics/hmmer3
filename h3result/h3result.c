#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "h3result.h"
#include "expect.h"
#include "lio.h"
#include "rc.h"
#include "struct.h"
#include "write.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static FILE *fdup(int fd, char const *mode)
{

  int nfd = dup(fd);
  if (nfd < 0) return NULL;
  FILE *fp = fdopen(nfd, mode);
  if (!fp)
  {
    close(nfd);
    return NULL;
  }
  return fp;
}

struct h3r *h3r_new(void)
{
  struct h3r *x = malloc(sizeof(*x));
  if (!x) return 0;
  h3r_stats_init(&x->stats);
  h3r_tophits_init(&x->tophits);
  return x;
}

void h3r_del(struct h3r const *x)
{
  if (x)
  {
    h3r_tophits_cleanup((struct tophits *)&x->tophits);
    free((void *)x);
  }
}

int h3r_pack(struct h3r const *x, int fd)
{
  struct lio_writer f = {0};
  lio_wsetup(&f, fd);

  if (write_map(&f, 1))              return H3R_EPACK;
  if (write_cstring(&f, "h3result")) return H3R_EPACK;

  if (write_map(&f, 2))              return H3R_EPACK;
  if (write_cstring(&f, "stats"))    return H3R_EPACK;
  int rc = h3r_stats_pack(&x->stats, &f);
  if (rc) return rc;

  if (write_cstring(&f, "tophits"))  return H3R_EPACK;
  return h3r_tophits_pack(&x->tophits, &f);
}

int h3r_unpack(struct h3r *x, int fd)
{
  struct lio_reader f = {0};
  lio_rsetup(&f, fd);

  if (expect_map(&f, 1))          return H3R_EUNPACK;

  if (expect_key(&f, "h3result")) return H3R_EUNPACK;

  if (expect_map(&f, 2))          return H3R_EUNPACK;
  if (expect_key(&f, "stats"))    return H3R_EUNPACK;
  int rc = h3r_stats_unpack(&x->stats, &f);
  if (rc) return rc;

  if (expect_key(&f, "tophits"))  return H3R_EUNPACK;
  return h3r_tophits_unpack(&x->tophits, &f);
}

int h3r_print_targets(struct h3r const *r, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3R_EPRINT;
  int rc = h3r_tophits_print_targets(&r->tophits, fp, r->stats.Z);
  if (rc)
  {
    fclose(fp);
    return rc;
  }
  return fclose(fp) ? H3R_EPRINT : 0;
}

int h3r_print_domains(struct h3r const *x, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3R_EPRINT;
  int rc = h3r_tophits_print_domains(&x->tophits, fp, x->stats.Z, x->stats.domZ);
  if (rc)
  {
    fclose(fp);
    return rc;
  }
  return fclose(fp) ? H3R_EPRINT : 0;
}

int h3r_print_targets_table(struct h3r const *x, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3R_EPRINT;
  int rc = h3r_tophits_print_targets_table("-", &x->tophits, fp, true, x->stats.Z);
  if (rc)
  {
    fclose(fp);
    return rc;
  }
  return fclose(fp) ? H3R_EPRINT : 0;
}

int h3r_print_domains_table(struct h3r const *r, int fd)
{
  FILE *fp = fdup(fd, "w");
  if (!fp) return H3R_EPRINT;
  int rc = h3r_tophits_print_domains_table("-", &r->tophits, fp, true,
                                           r->stats.Z, r->stats.domZ);
  if (rc)
  {
    fclose(fp);
    return rc;
  }
  return fclose(fp) ? H3R_EPRINT : 0;
}

unsigned h3r_nhits(struct h3r const *r) { return r->tophits.nhits; }

char const *h3r_hit_name(struct h3r const *x, unsigned idx)
{
  return h3r_tophits_hit_name(&x->tophits, idx);
}

char const *h3r_hit_accession(struct h3r const *x, unsigned idx)
{
  return h3r_tophits_hit_accession(&x->tophits, idx);
}

double h3r_hit_logevalue(struct h3r const *x, unsigned idx)
{
  return h3r_tophits_hit_logevalue(&x->tophits, idx, x->stats.Z);
}

static char const *msg[] = {
    [H3R_EUNPACK] = "unpack failure",
    [H3R_EPACK]   = "pack failure",
    [H3R_ENOMEM]  = "insufficient free memory",
    [H3R_EPRINT]  = "print failure",
};

#define array_size(arr) (sizeof(arr) / sizeof((arr)[0]))

char const *h3r_strerror(int rc)
{
  if (rc > 0 && rc < (int)array_size(msg)) return msg[rc];

  static char unknown[32] = {0};
  snprintf(unknown, sizeof unknown, "unknown error #%d", rc);
  return unknown;
}
