#ifndef H3R_RESULT_H
#define H3R_RESULT_H

#include "h3r_stats.h"
#include "h3r_tophits.h"

struct h3r
{
  struct h3r_stats stats;
  struct h3r_tophits tophits;
};

struct h3r *h3r_new(void);
void        h3r_del(struct h3r const *);

int         h3r_pack(struct h3r const *, int fd);
int         h3r_unpack(struct h3r *, int fd);

int         h3r_print_targets(struct h3r const *, int fd);
int         h3r_print_domains(struct h3r const *, int fd);

int         h3r_print_targets_table(struct h3r const *, int fd);
int         h3r_print_domains_table(struct h3r const *, int fd);

unsigned    h3r_nhits(struct h3r const *);
char const *h3r_hit_name(struct h3r const *, unsigned idx);
char const *h3r_hit_accession(struct h3r const *, unsigned idx);
double      h3r_hit_logevalue(struct h3r const *, unsigned idx);

char const *h3r_strerror(int rc);

#endif
