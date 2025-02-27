#include "h3r_alidisplay.h"
#include "defer_return.h"
#include "echo.h"
#include "expect.h"
#include "lio.h"
#include "max.h"
#include "rc.h"
#include "read.h"
#include "static_assert2.h"
#include "write.h"
#include "zero_clip.h"
#include <stdlib.h>
#include <string.h>

static_assert2(sizeof(unsigned) >= 4);

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT (1 << 4)
#define NTSEQ_PRESENT (1 << 5)

static inline void unset(struct h3r_alidisplay *x)
{
  x->presence = 0;

  x->rfline   = NULL;
  x->mmline   = NULL;
  x->csline   = NULL;
  x->model    = NULL;
  x->mline    = NULL;
  x->aseq     = NULL;
  x->ntseq    = NULL;
  x->ppline   = NULL;
  x->N        = 0;

  x->hmmname  = NULL;
  x->hmmacc   = NULL;
  x->hmmdesc  = NULL;
  x->hmmfrom  = 0;
  x->hmmto    = 0;
  x->M        = 0;

  x->sqname   = NULL;
  x->sqacc    = NULL;
  x->sqdesc   = NULL;
  x->sqfrom   = 0;
  x->sqto     = 0;
  x->L        = 0;
}

int h3r_alidisplay_init(struct h3r_alidisplay *x)
{
  int rc = 0;
  unset(x);

  if (!(x->rfline = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);
  if (!(x->mmline = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);
  if (!(x->csline = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);
  if (!(x->model = malloc(sizeof(char))))   defer_return(H3R_ENOMEM);
  if (!(x->mline = malloc(sizeof(char))))   defer_return(H3R_ENOMEM);
  if (!(x->aseq = malloc(sizeof(char))))    defer_return(H3R_ENOMEM);
  if (!(x->ntseq = malloc(sizeof(char))))   defer_return(H3R_ENOMEM);
  if (!(x->ppline = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);

  if (!(x->hmmname = malloc(sizeof(char)))) defer_return(H3R_ENOMEM);
  if (!(x->hmmacc = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);
  if (!(x->hmmdesc = malloc(sizeof(char)))) defer_return(H3R_ENOMEM);

  if (!(x->sqname = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);
  if (!(x->sqacc = malloc(sizeof(char))))   defer_return(H3R_ENOMEM);
  if (!(x->sqdesc = malloc(sizeof(char))))  defer_return(H3R_ENOMEM);

  x->rfline[0] = '\0';
  x->mmline[0] = '\0';
  x->csline[0] = '\0';
  x->model[0]  = '\0';
  x->mline[0]  = '\0';
  x->aseq[0]   = '\0';
  x->ntseq[0]  = '\0';
  x->ppline[0] = '\0';

  x->hmmname[0] = '\0';
  x->hmmacc[0]  = '\0';
  x->hmmdesc[0] = '\0';

  x->sqname[0] = '\0';
  x->sqacc[0]  = '\0';
  x->sqdesc[0] = '\0';

  return 0;

defer:
  h3r_alidisplay_cleanup(x);
  return rc;
}

void h3r_alidisplay_cleanup(struct h3r_alidisplay *x)
{
  free(x->rfline);
  free(x->mmline);
  free(x->csline);
  free(x->model);
  free(x->mline);
  free(x->aseq);
  free(x->ntseq);
  free(x->ppline);

  free(x->hmmname);
  free(x->hmmacc);
  free(x->hmmdesc);

  free(x->sqname);
  free(x->sqacc);
  free(x->sqdesc);

  unset(x);
}

int h3r_alidisplay_pack(struct h3r_alidisplay const *x, struct lio_writer *f)
{
  if (write_array(f, 22))                                              return H3R_EPACK;

  if (write_int(f, x->presence))                                       return H3R_EPACK;
  if (write_cstring(f, x->presence & RFLINE_PRESENT ? x->rfline : "")) return H3R_EPACK;
  if (write_cstring(f, x->presence & MMLINE_PRESENT ? x->mmline : "")) return H3R_EPACK;
  if (write_cstring(f, x->presence & CSLINE_PRESENT ? x->csline : "")) return H3R_EPACK;
  if (write_cstring(f, x->model))                                      return H3R_EPACK;
  if (write_cstring(f, x->mline))                                      return H3R_EPACK;
  if (write_cstring(f, x->presence & ASEQ_PRESENT ? x->aseq : ""))     return H3R_EPACK;
  if (write_cstring(f, x->presence & NTSEQ_PRESENT ? x->ntseq : ""))   return H3R_EPACK;
  if (write_cstring(f, x->presence & PPLINE_PRESENT ? x->ppline : "")) return H3R_EPACK;
  if (write_int(f, x->N))                                              return H3R_EPACK;

  if (write_cstring(f, x->hmmname))                                    return H3R_EPACK;
  if (write_cstring(f, x->hmmacc))                                     return H3R_EPACK;
  if (write_cstring(f, x->hmmdesc))                                    return H3R_EPACK;
  if (write_int(f, x->hmmfrom))                                        return H3R_EPACK;
  if (write_int(f, x->hmmto))                                          return H3R_EPACK;
  if (write_int(f, x->M))                                              return H3R_EPACK;

  if (write_cstring(f, x->sqname))                                     return H3R_EPACK;
  if (write_cstring(f, x->sqacc))                                      return H3R_EPACK;
  if (write_cstring(f, x->sqdesc))                                     return H3R_EPACK;
  if (write_int(f, x->sqfrom))                                         return H3R_EPACK;
  if (write_int(f, x->sqto))                                           return H3R_EPACK;
  if (write_int(f, x->L))                                              return H3R_EPACK;

  return 0;
}

int h3r_alidisplay_unpack(struct h3r_alidisplay *x, struct lio_reader *f)
{
  int rc = 0;

  if (expect_array(f, 22))                  defer_return(H3R_ENOMEM);

  if (read_int(f, &x->presence))            defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->rfline)))  defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->mmline)))  defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->csline)))  defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->model)))   defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->mline)))   defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->aseq)))    defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->ntseq)))   defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->ppline)))  defer_return(H3R_ENOMEM);
  if (read_int(f, &x->N))                   defer_return(H3R_ENOMEM);

  if ((rc = read_cstring2(f, &x->hmmname))) defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->hmmacc)))  defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->hmmdesc))) defer_return(H3R_ENOMEM);
  if (read_int(f, &x->hmmfrom))             defer_return(H3R_ENOMEM);
  if (read_int(f, &x->hmmto))               defer_return(H3R_ENOMEM);
  if (read_int(f, &x->M))                   defer_return(H3R_ENOMEM);

  if ((rc = read_cstring2(f, &x->sqname)))  defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->sqacc)))   defer_return(H3R_ENOMEM);
  if ((rc = read_cstring2(f, &x->sqdesc)))  defer_return(H3R_ENOMEM);
  if (read_int(f, &x->sqfrom))              defer_return(H3R_ENOMEM);
  if (read_int(f, &x->sqto))                defer_return(H3R_ENOMEM);
  if (read_int(f, &x->L))                   defer_return(H3R_ENOMEM);

  return 0;

defer:
  h3r_alidisplay_cleanup(x);
  return rc;
}

static unsigned textwidth(unsigned n)
{
  unsigned w = 0;
  while (n != 0)
  {
    n /= 10;
    w++;
  }
  return w;
}

int h3r_alidisplay_print(struct h3r_alidisplay const *x, FILE *f)
{
  /* implement the --acc option for preferring accessions over names in output
   */
  char const *hmmname = x->hmmacc[0] != 0 ? x->hmmacc : x->hmmname;
  char const *seqname = x->sqacc[0] != 0 ? x->sqacc : x->sqname;

  /* dynamically size the output lines */
  unsigned namewidth = max(strlen(hmmname), strlen(seqname));
  unsigned coordwidth = max(max(textwidth(x->hmmfrom), textwidth(x->hmmto)),
                            max(textwidth(x->sqfrom), textwidth(x->sqto)));

  unsigned aliwidth = zero_clip(120 - namewidth - 2 * coordwidth - 5);
  if (aliwidth < x->N && aliwidth < 40) aliwidth = 40;

  char buf[121] = {0};

  /* Break the alignment into multiple blocks of width aliwidth for printing
   */
  unsigned i1 = x->sqfrom;
  unsigned k1 = x->hmmfrom;
  for (unsigned pos = 0; pos < x->N; pos += aliwidth)
  {
    if (pos > 0)
    {
      if (echon(f, "%s", "")) return H3R_EPRINT;
    }

    unsigned ni = 0;
    unsigned nk = 0;
    for (unsigned z = pos; z < pos + aliwidth && z < x->N; z++)
    {
      if (x->model[z] != '.') nk++; /* k advances except on insert states */
      if (x->aseq[z] != '-') ni++;  /* i advances except on delete states */
    }

    unsigned k2 = k1 + nk - 1;
    unsigned i2 = 0;
    if (x->sqfrom < x->sqto)
      i2 = (unsigned)(i1 + ni - 1);
    else
      i2 = (unsigned)(i1 - ni + 1); // revcomp hit for DNA

    if (x->presence & CSLINE_PRESENT)
    {
      strncpy(buf, x->csline + pos, aliwidth);
      if (echon(f, "  %*s %s CS", namewidth + coordwidth + 1, "", buf)) return H3R_EPRINT;
    }
    if (x->presence & RFLINE_PRESENT)
    {
      strncpy(buf, x->rfline + pos, aliwidth);
      if (echon(f, "  %*s %s RF", namewidth + coordwidth + 1, "", buf)) return H3R_EPRINT;
    }
    if (x->presence & MMLINE_PRESENT)
    {
      strncpy(buf, x->mmline + pos, aliwidth);
      if (echon(f, "  %*s %s MM", namewidth + coordwidth + 1, "", buf)) return H3R_EPRINT;
    }

    strncpy(buf, x->model + pos, aliwidth);
    if (echon(f, "  %*s %*d %s %-*d", namewidth, hmmname, coordwidth, k1, buf,
              coordwidth, k2)) return H3R_EPRINT;

    strncpy(buf, x->mline + pos, aliwidth);
    if (echon(f, "  %*s %s", namewidth + coordwidth + 1, " ", buf)) return H3R_EPRINT;

    strncpy(buf, x->aseq + pos, aliwidth);
    if (ni > 0)
    {
      if (echon(f, "  %*s %*u %s %-*u", namewidth, seqname, coordwidth, i1, buf,
                coordwidth, i2)) return H3R_EPRINT;
    }
    else
    {
      if (echon(f, "  %*s %*s %s %*s", namewidth, seqname, coordwidth, "-", buf,
                coordwidth, "-")) return H3R_EPRINT;
    }

    if (x->ppline != 0)
    {
      strncpy(buf, x->ppline + pos, aliwidth);
      if (echon(f, "  %*s %s PP", namewidth + coordwidth + 1, "", buf)) return H3R_EPRINT;
    }

    k1 += nk;
    if (x->sqfrom < x->sqto)
      i1 += ni;
    else
      i1 -= ni; // revcomp hit for DNA
  }
  return 0;
}
