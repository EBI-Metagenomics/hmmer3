#ifndef H3RESULT_TOPHITS_H
#define H3RESULT_TOPHITS_H

#include "static_assert2.h"
#include <stdbool.h>
#include <stdio.h>

struct tophits;
struct lio_writer;
struct lio_reader;

struct tophits
{
  unsigned nhits;
  struct hit *hits;
  unsigned nreported;
  unsigned nincluded;
  bool is_sorted_by_sortkey;
  bool is_sorted_by_seqidx;
};

static_assert2(sizeof(unsigned) >= 4);

void h3r_tophits_init(struct tophits *);
int h3r_tophits_setup(struct tophits *, unsigned nhits);
void h3r_tophits_cleanup(struct tophits *);

int h3r_tophits_pack(struct tophits const *, struct lio_writer *);
int h3r_tophits_unpack(struct tophits *, struct lio_reader *);

void h3r_tophits_print_targets(struct tophits const *, FILE *, double Z);
void h3r_tophits_print_domains(struct tophits const *, FILE *, double Z,
                               double domZ);

void h3r_tophits_print_targets_table(char const *qacc, struct tophits const *th,
                                     FILE *, bool show_header, double Z);

void h3r_tophits_print_domains_table(char const *qacc, struct tophits const *th,
                                     FILE *, bool show_header, double Z,
                                     double domZ);

char const *h3r_tophits_hit_name(struct tophits const *th, unsigned idx);
char const *h3r_tophits_hit_acc(struct tophits const *th, unsigned idx);
double h3r_tophits_hit_evalue_ln(struct tophits const *th, unsigned idx,
                                 double Z);

#endif
