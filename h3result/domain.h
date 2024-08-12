#ifndef DOMAIN_H
#define DOMAIN_H

#include "alidisplay.h"
#include "static_assert2.h"
#include <stdbool.h>
#include <stdint.h>

struct domain;
struct lio_writer;
struct lio_reader;

struct domain
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
  struct alidisplay ad;
};

static_assert2(sizeof(long) >= 8);

int  h3r_domain_init(struct domain *);
int  h3r_domain_setup(struct domain *, unsigned scores_size);
void h3r_domain_cleanup(struct domain *);
int  h3r_domain_pack(struct domain const *, struct lio_writer *);
int  h3r_domain_unpack(struct domain *, struct lio_reader *);

#endif
