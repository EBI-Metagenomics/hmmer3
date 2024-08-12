#ifndef H3RESULT_DOMAIN_H
#define H3RESULT_DOMAIN_H

#include "alidisplay.h"
#include "static_assert.h"
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

h3result_static_assert(sizeof(long) >= 8);

int h3result_domain_init(struct domain *);
int h3result_domain_setup(struct domain *, unsigned scores_size);
void h3result_domain_cleanup(struct domain *);
int h3result_domain_pack(struct domain const *, struct lio_writer *);
int h3result_domain_unpack(struct domain *, struct lio_reader *);

#endif
