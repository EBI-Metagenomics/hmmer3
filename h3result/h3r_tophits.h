#ifndef H3R_TOPHITS_H
#define H3R_TOPHITS_H

#include <stdbool.h>
#include <stdio.h>

struct h3r_tophits;
struct lio_writer;
struct lio_reader;

struct h3r_tophits
{
  unsigned nhits;
  struct h3r_hit *hits;
  unsigned nreported;
  unsigned nincluded;
  bool is_sorted_by_sortkey;
  bool is_sorted_by_seqidx;
};

void h3r_tophits_init(struct h3r_tophits *);
int  h3r_tophits_setup(struct h3r_tophits *, unsigned nhits);
void h3r_tophits_cleanup(struct h3r_tophits *);

int h3r_tophits_pack(struct h3r_tophits const *, struct lio_writer *);
int h3r_tophits_unpack(struct h3r_tophits *, struct lio_reader *);

int h3r_tophits_print_targets(struct h3r_tophits const *, FILE *, double Z);
int h3r_tophits_print_domains(struct h3r_tophits const *, FILE *, double Z,
                              double domZ);

int h3r_tophits_print_targets_table(char const *qacc, struct h3r_tophits const *,
                                    FILE *, bool show_header, double Z);

int h3r_tophits_print_domains_table(char const *qacc, struct h3r_tophits const *,
                                    FILE *, bool show_header, double Z,
                                    double domZ);

char const *h3r_tophits_hit_name(struct h3r_tophits const *, unsigned idx);
char const *h3r_tophits_hit_accession(struct h3r_tophits const *, unsigned idx);
double      h3r_tophits_hit_logevalue(struct h3r_tophits const *, unsigned idx, double Z);

#endif
