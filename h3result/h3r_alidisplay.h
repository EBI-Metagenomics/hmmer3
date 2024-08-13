#ifndef H3R_ALIDISPLAY_H
#define H3R_ALIDISPLAY_H

#include <stdio.h>

struct lio_writer;
struct lio_reader;

struct h3r_alidisplay
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

int  h3r_alidisplay_init(struct h3r_alidisplay *);
void h3r_alidisplay_cleanup(struct h3r_alidisplay *);
int  h3r_alidisplay_pack(struct h3r_alidisplay const *, struct lio_writer *);
int  h3r_alidisplay_unpack(struct h3r_alidisplay *, struct lio_reader *);
int  h3r_alidisplay_print(struct h3r_alidisplay const *, FILE *);

#endif
