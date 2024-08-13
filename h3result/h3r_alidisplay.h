#ifndef H3R_ALIDISPLAY_H
#define H3R_ALIDISPLAY_H

#include "static_assert2.h"
#include <stdio.h>

struct lio_writer;
struct lio_reader;

struct alidisplay
{
  unsigned presence;

  char *rfline;
  char *mmline;
  char *csline;
  char *model;
  char *mline;
  char *aseq;
  char *ntseq;
  char *ppline;
  unsigned N;

  char *hmmname;
  char *hmmacc;
  char *hmmdesc;
  unsigned hmmfrom;
  unsigned hmmto;
  unsigned M;

  char *sqname;
  char *sqacc;
  char *sqdesc;
  unsigned sqfrom;
  unsigned sqto;
  unsigned L;
};

static_assert2(sizeof(unsigned) >= 4);

int  h3r_alidisplay_init(struct alidisplay *);
void h3r_alidisplay_cleanup(struct alidisplay *);
int  h3r_alidisplay_pack(struct alidisplay const *, struct lio_writer *);
int  h3r_alidisplay_unpack(struct alidisplay *, struct lio_reader *);
int  h3r_alidisplay_print(struct alidisplay const *, FILE *);

#endif
