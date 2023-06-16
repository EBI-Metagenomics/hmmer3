#ifndef HMMD_ALIDISPLAY_H
#define HMMD_ALIDISPLAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hmmd_alidisplay
{
  uint8_t presence;

  char *rfline;
  char *mmline;
  char *csline;
  char *model;
  char *mline;
  char *aseq;
  char *ntseq;
  char *ppline;
  uint32_t N;

  char *hmmname;
  char *hmmacc;
  char *hmmdesc;
  uint32_t hmmfrom;
  uint32_t hmmto;
  uint32_t M;

  char *sqname;
  char *sqacc;
  char *sqdesc;
  uint64_t sqfrom;
  uint64_t sqto;
  uint64_t L;

  size_t memsize;
  char *mem;
};

void h3client_hmmd_alidisplay_init(struct hmmd_alidisplay *);
void h3client_hmmd_alidisplay_cleanup(struct hmmd_alidisplay *);
int h3client_hmmd_alidisplay_parse(struct hmmd_alidisplay *,
                                   unsigned char const **ptr,
                                   unsigned char const *end);

#endif
