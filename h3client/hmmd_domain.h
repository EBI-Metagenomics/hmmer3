#ifndef HMMD_DOMAIN_H
#define HMMD_DOMAIN_H

#include "hmmd_alidisplay.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hmmd_domain
{
  uint32_t ienv;
  uint32_t jenv;
  uint32_t iali;
  uint32_t jali;
  // The following two members seems to be receiving
  // random numbers from daemon end. Skip them.
  // int64_t iorf, jorf;
  float envsc;
  float domcorrection;
  float dombias;
  float oasc;
  float bitscore;
  double lnP;
  bool is_reported;
  bool is_included;
  uint32_t scores_size;
  float *pos_score;
  struct hmmd_alidisplay ad;
};

void h3client_hmmd_domain_init(struct hmmd_domain *);
void h3client_hmmd_domain_cleanup(struct hmmd_domain *);
int h3client_hmmd_domain_parse(struct hmmd_domain *, unsigned char const **ptr,
                               unsigned char const *end);

#endif
