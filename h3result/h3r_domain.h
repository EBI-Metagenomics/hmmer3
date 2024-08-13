#ifndef H3R_DOMAIN_H
#define H3R_DOMAIN_H

#include "h3r_alidisplay.h"
#include <stdbool.h>
#include <stdint.h>

struct h3r_domain;
struct lio_writer;
struct lio_reader;

struct h3r_domain
{
  unsigned long ienv;
  unsigned long jenv;
  unsigned long iali;
  unsigned long jali;
  float envsc;
  float domcorrection;
  float dombias;
  float oasc;
  float bitscore;
  double lnP;
  bool is_reported;
  bool is_included;
  unsigned pos_score_size;
  float *pos_score;
  struct h3r_alidisplay ad;
};

int  h3r_domain_init(struct h3r_domain *);
int  h3r_domain_setup(struct h3r_domain *, unsigned scores_size);
void h3r_domain_cleanup(struct h3r_domain *);
int  h3r_domain_pack(struct h3r_domain const *, struct lio_writer *);
int  h3r_domain_unpack(struct h3r_domain *, struct lio_reader *);

#endif
