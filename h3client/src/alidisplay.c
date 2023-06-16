#include "alidisplay.h"
#include "echo.h"
#include "h3client/errno.h"
#include "lip/lip.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT (1 << 4)
#define NTSEQ_PRESENT (1 << 5)

int h3client_alidisplay_init(struct alidisplay *ad)
{
  memset(ad, 0, sizeof(*ad));

  if (!(ad->rfline = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->mmline = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->csline = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->model = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->mline = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->aseq = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->ntseq = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->ppline = calloc(1, sizeof(char)))) goto cleanup;

  if (!(ad->hmmname = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->hmmacc = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->hmmdesc = calloc(1, sizeof(char)))) goto cleanup;

  if (!(ad->sqname = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->sqacc = calloc(1, sizeof(char)))) goto cleanup;
  if (!(ad->sqdesc = calloc(1, sizeof(char)))) goto cleanup;

  return 0;

cleanup:
  h3client_alidisplay_cleanup(ad);
  return H3CLIENT_ENOMEM;
}

void h3client_alidisplay_cleanup(struct alidisplay *ad)
{
  DEL(ad->rfline);
  DEL(ad->mmline);
  DEL(ad->csline);
  DEL(ad->model);
  DEL(ad->mline);
  DEL(ad->aseq);
  DEL(ad->ntseq);
  DEL(ad->ppline);

  DEL(ad->hmmname);
  DEL(ad->hmmacc);
  DEL(ad->hmmdesc);

  DEL(ad->sqname);
  DEL(ad->sqacc);
  DEL(ad->sqdesc);
}

static void write_cstr(bool presence, struct lip_file *f, char const *str)
{
  if (presence)
    lip_write_cstr(f, str);
  else
    lip_write_cstr(f, "");
}

int h3client_alidisplay_pack(struct alidisplay const *ad, struct lip_file *f)
{
  lip_write_array_size(f, 22);

  lip_write_int(f, ad->presence);
  write_cstr(ad->presence & RFLINE_PRESENT, f, ad->rfline);
  write_cstr(ad->presence & MMLINE_PRESENT, f, ad->mmline);
  write_cstr(ad->presence & CSLINE_PRESENT, f, ad->csline);
  lip_write_cstr(f, ad->model);
  lip_write_cstr(f, ad->mline);
  write_cstr(ad->presence & ASEQ_PRESENT, f, ad->aseq);
  write_cstr(ad->presence & NTSEQ_PRESENT, f, ad->ntseq);
  write_cstr(ad->presence & PPLINE_PRESENT, f, ad->ppline);
  lip_write_int(f, ad->N);

  lip_write_cstr(f, ad->hmmname);
  lip_write_cstr(f, ad->hmmacc);
  lip_write_cstr(f, ad->hmmdesc);
  lip_write_int(f, ad->hmmfrom);
  lip_write_int(f, ad->hmmto);
  lip_write_int(f, ad->M);

  lip_write_cstr(f, ad->sqname);
  lip_write_cstr(f, ad->sqacc);
  lip_write_cstr(f, ad->sqdesc);
  lip_write_int(f, ad->sqfrom);
  lip_write_int(f, ad->sqto);
  lip_write_int(f, ad->L);

  return lip_file_error(f) ? H3CLIENT_EPACK : 0;
}

int h3client_alidisplay_unpack(struct alidisplay *ad, struct lip_file *f)
{
  int rc = H3CLIENT_EUNPACK;

  if (!h3client_expect_array_size(f, 22)) goto cleanup;

  if (!lip_read_int(f, &ad->presence)) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->rfline))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->mmline))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->csline))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->model))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->mline))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->aseq))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->ntseq))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->ppline))) goto cleanup;
  lip_read_int(f, &ad->N);

  if ((rc = h3client_read_string(f, &ad->hmmname))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->hmmacc))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->hmmdesc))) goto cleanup;
  lip_read_int(f, &ad->hmmfrom);
  lip_read_int(f, &ad->hmmto);
  lip_read_int(f, &ad->M);

  if ((rc = h3client_read_string(f, &ad->sqname))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->sqacc))) goto cleanup;
  if ((rc = h3client_read_string(f, &ad->sqdesc))) goto cleanup;
  lip_read_int(f, &ad->sqfrom);
  lip_read_int(f, &ad->sqto);
  lip_read_int(f, &ad->L);

  rc = H3CLIENT_EUNPACK;
  if (lip_file_error(f)) goto cleanup;

  return 0;

cleanup:
  h3client_alidisplay_cleanup(ad);
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

void h3client_alidisplay_print(struct alidisplay const *ad, FILE *f)
{
  /* implement the --acc option for preferring accessions over names in output
   */
  char const *hmmname = ad->hmmacc[0] != 0 ? ad->hmmacc : ad->hmmname;
  char const *seqname = ad->sqacc[0] != 0 ? ad->sqacc : ad->sqname;

  /* dynamically size the output lines */
  unsigned namewidth = MAX(strlen(hmmname), strlen(seqname));
  unsigned coordwidth = MAX(MAX(textwidth(ad->hmmfrom), textwidth(ad->hmmto)),
                            MAX(textwidth(ad->sqfrom), textwidth(ad->sqto)));

  unsigned aliwidth = zero_clip(120 - namewidth - 2 * coordwidth - 5);
  if (aliwidth < ad->N && aliwidth < 40) aliwidth = 40;

  char buf[121] = {0};

  /* Break the alignment into multiple blocks of width aliwidth for printing
   */
  unsigned i1 = ad->sqfrom;
  unsigned k1 = ad->hmmfrom;
  for (unsigned pos = 0; pos < ad->N; pos += aliwidth)
  {
    if (pos > 0) h3client_echo(f, "");

    unsigned ni = 0;
    unsigned nk = 0;
    for (unsigned z = pos; z < pos + aliwidth && z < ad->N; z++)
    {
      if (ad->model[z] != '.') nk++; /* k advances except on insert states */
      if (ad->aseq[z] != '-') ni++;  /* i advances except on delete states */
    }

    unsigned k2 = k1 + nk - 1;
    unsigned i2 = 0;
    if (ad->sqfrom < ad->sqto)
      i2 = (unsigned)(i1 + ni - 1);
    else
      i2 = (unsigned)(i1 - ni + 1); // revcomp hit for DNA

    if (ad->presence & CSLINE_PRESENT)
    {
      strncpy(buf, ad->csline + pos, aliwidth);
      h3client_echo(f, "  %*s %s CS", namewidth + coordwidth + 1, "", buf);
    }
    if (ad->presence & RFLINE_PRESENT)
    {
      strncpy(buf, ad->rfline + pos, aliwidth);
      h3client_echo(f, "  %*s %s RF", namewidth + coordwidth + 1, "", buf);
    }
    if (ad->presence & MMLINE_PRESENT)
    {
      strncpy(buf, ad->mmline + pos, aliwidth);
      h3client_echo(f, "  %*s %s MM", namewidth + coordwidth + 1, "", buf);
    }

    strncpy(buf, ad->model + pos, aliwidth);
    h3client_echo(f, "  %*s %*d %s %-*d", namewidth, hmmname, coordwidth, k1,
                  buf, coordwidth, k2);

    strncpy(buf, ad->mline + pos, aliwidth);
    h3client_echo(f, "  %*s %s", namewidth + coordwidth + 1, " ", buf);

    strncpy(buf, ad->aseq + pos, aliwidth);
    if (ni > 0)
    {
      h3client_echo(f, "  %*s %*ld %s %-*ld", namewidth, seqname, coordwidth,
                    i1, buf, coordwidth, i2);
    }
    else
    {
      h3client_echo(f, "  %*s %*s %s %*s", namewidth, seqname, coordwidth, "-",
                    buf, coordwidth, "-");
    }

    if (ad->ppline != 0)
    {
      strncpy(buf, ad->ppline + pos, aliwidth);
      h3client_echo(f, "  %*s %s PP", namewidth + coordwidth + 1, "", buf);
    }

    k1 += nk;
    if (ad->sqfrom < ad->sqto)
      i1 += ni;
    else
      i1 -= ni; // revcomp hit for DNA
  }
}
