#include "tophits.h"
#include "domain.h"
#include "echo.h"
#include "expect.h"
#include "hit.h"
#include "lio.h"
#include "max.h"
#include "rc.h"
#include "read.h"
#include "tophits.h"
#include "write.h"
#include "zero_clip.h"
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void h3result_tophits_init(struct tophits *th) { memset(th, 0, sizeof(*th)); }

static int grow(struct tophits *th, unsigned nhits)
{
  int rc = 0;

  size_t sz = nhits * sizeof(*th->hits);
  struct hit *hits = realloc(th->hits, sz);
  if (!hits)
  {
    rc = H3RESULT_ENOMEM;
    goto cleanup;
  }
  th->hits = hits;

  for (unsigned i = th->nhits; i < nhits; ++i)
  {
    if ((rc = h3result_hit_init(th->hits + i))) goto cleanup;
    ++th->nhits;
  }

  return 0;

cleanup:
  h3result_tophits_cleanup(th);
  return rc;
}

static void shrink(struct tophits *th, unsigned nhits)
{
  for (unsigned i = nhits; i < th->nhits; ++i)
    h3result_hit_cleanup(th->hits + i);

  th->nhits = nhits;
}

int h3result_tophits_setup(struct tophits *th, unsigned nhits)
{
  if (th->nhits < nhits) return grow(th, nhits);
  shrink(th, nhits);
  return 0;
}

void h3result_tophits_cleanup(struct tophits *th)
{
  for (unsigned i = 0; i < th->nhits; ++i)
    h3result_hit_cleanup(th->hits + i);
  free(th->hits);
  th->hits = NULL;
  th->nhits = 0;
}

int h3result_tophits_pack(struct tophits const *th, struct lio_writer *f)
{
  if (write_array(f, 5)) return H3RESULT_EPACK;

  if (write_map(f, 1)) return H3RESULT_EPACK;
  if (write_cstring(f, "hits")) return H3RESULT_EPACK;
  if (write_array(f, th->nhits)) return H3RESULT_EPACK;

  for (unsigned i = 0; i < th->nhits; ++i)
  {
    int rc = h3result_hit_pack(th->hits + i, f);
    if (rc) return rc;
  }

  if (write_int(f, th->nreported)) return H3RESULT_EPACK;
  if (write_int(f, th->nincluded)) return H3RESULT_EPACK;
  if (write_bool(f, th->is_sorted_by_sortkey)) return H3RESULT_EPACK;
  if (write_bool(f, th->is_sorted_by_seqidx)) return H3RESULT_EPACK;

  return 0;
}

int h3result_tophits_unpack(struct tophits *th, struct lio_reader *f)
{
  int rc = H3RESULT_EUNPACK;

  if (!expect_array(f, 5)) goto cleanup;

  if (!expect_map(f, 1)) goto cleanup;
  if (!expect_key(f, "hits")) goto cleanup;

  unsigned size = 0;
  if (read_array(f, &size)) goto cleanup;

  if ((rc = h3result_tophits_setup(th, size))) goto cleanup;

  for (unsigned i = 0; i < th->nhits; ++i)
  {
    if ((rc = h3result_hit_unpack(th->hits + i, f))) goto cleanup;
  }

  if (read_int(f, &th->nreported)) goto cleanup;
  if (read_int(f, &th->nincluded)) goto cleanup;
  if (read_bool(f, &th->is_sorted_by_sortkey)) goto cleanup;
  if (read_bool(f, &th->is_sorted_by_seqidx)) goto cleanup;

  return 0;

cleanup:
  h3result_tophits_cleanup(th);
  return rc;
}

static unsigned max_shown_length(struct tophits const *h)
{
  unsigned max = 0;
  for (unsigned i = 0; i < h->nhits; i++)
  {
    max = max(max, (unsigned)strlen(h->hits[i].acc));
    max = max(max, (unsigned)strlen(h->hits[i].name));
  }
  return max;
}

static unsigned max_name_length(struct tophits const *h)
{
  unsigned max = 0;
  for (unsigned i = 0; i < h->nhits; i++)
    max = max(max, (unsigned)strlen(h->hits[i].name));
  return max;
}

#define p7_HITFLAGS_DEFAULT 0
#define p7_IS_INCLUDED (1 << 0)
#define p7_IS_REPORTED (1 << 1)
#define p7_IS_NEW (1 << 2)
#define p7_IS_DROPPED (1 << 3)
#define p7_IS_DUPLICATE (1 << 4)

static inline char const *show_name(struct hit const *hit)
{
  return (hit->acc != 0 && hit->acc[0] != '\0') ? hit->acc : hit->name;
}

static inline char const *strdash(char const *str)
{
  static char const dash[] = "-";
  return strlen(str) == 0 ? dash : str;
}

static inline char newness(struct hit const *hit)
{
  char symbol = ' ';
  if (hit->flags & p7_IS_NEW)
    symbol = '+';
  else if (hit->flags & p7_IS_DROPPED)
    symbol = '-';
  return symbol;
}

#define CONST_LOG2R 1.44269504088896341

static inline double dombits(struct domain const *dom)
{
  return CONST_LOG2R * dom->dombias;
}

static inline float unbiased_score(struct hit const *hit)
{
  return hit->pre_score - hit->score;
}

static inline double evalue(double lnP, double Z) { return exp(lnP) * Z; }
static inline double evalue_ln(double lnP, double Z) { return lnP + log(Z); }

void h3result_tophits_print_targets(struct tophits const *x, FILE *fp, double Z)
{
  unsigned namew = max(8U, max_shown_length(x));
  unsigned descw = max(32U, h3result_zero_clip(120 - namew - 61));

  h3result_echo(fp,
                "Scores for complete sequence (score includes all domains):");

  h3result_echo(fp, "  %22s  %22s  %8s", " --- full sequence ---",
                " --- best 1 domain ---", "-#dom-");

  h3result_echo(fp, "  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s", "E-value",
                " score", " bias", "E-value", " score", " bias", "  exp", "N",
                namew, "Model", "Description");

  h3result_echo(fp, "  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s", "-------",
                "------", "-----", "-------", "------", "-----", " ----", "--",
                namew, "--------", "-----------");

  bool printed_incthresh = false;
  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;
    if (!(hit->flags & p7_IS_REPORTED)) continue;

    struct domain const *dom = hit->domains + hit->best_domain;

    if (!(hit->flags & p7_IS_INCLUDED) && !printed_incthresh)
    {
      h3result_echo(fp, "  ------ inclusion threshold ------");
      printed_incthresh = true;
    }

    h3result_echo(fp,
                  "%c %9.2g %6.1f %5.1f  %9.2g %6.1f %5.1f  %5.1f %2d  %-*s  "
                  "%-.*s",
                  newness(hit), evalue(hit->lnP, Z), hit->score,
                  unbiased_score(hit), evalue(dom->lnP, Z), dom->bitscore,
                  dombits(dom), hit->nexpected, hit->nreported, namew,
                  show_name(hit), descw, hit->desc);
  }

  if (x->nreported == 0)
    h3result_echo(fp,
                  "\n   [No hits detected that satisfy reporting thresholds]");
}

#define p7_HITFLAGS_DEFAULT 0
#define p7_IS_INCLUDED (1 << 0)
#define p7_IS_REPORTED (1 << 1)
#define p7_IS_NEW (1 << 2)
#define p7_IS_DROPPED (1 << 3)
#define p7_IS_DUPLICATE (1 << 4)

static void print_range(FILE *f, unsigned from, unsigned to, unsigned length)
{
  fprintf(f, " %7u %7u %c%c", from, to, from == 1 ? '[' : '.',
          to == length ? ']' : '.');
}

static double prob_ali_res(struct domain const *dom)
{
  return dom->oasc / (1.0 + fabs((float)(dom->jenv - dom->ienv)));
}

static char included_symbol(struct domain const *dom)
{
  return dom->is_included ? '!' : '?';
}

void h3result_tophits_print_domains(struct tophits const *x, FILE *f, double Z,
                                    double domZ)
{
  h3result_echo(f, "Domain annotation for each model (and alignments):");

  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;
    if (!(hit->flags & p7_IS_REPORTED)) continue;

    char const *name = show_name(hit);
    unsigned namew = (unsigned)strlen(name);
    unsigned descw = max(32U, h3result_zero_clip(120 - namew - 5));

    h3result_echo(f, ">> %s  %-.*s", name, descw, hit->desc);

    if (hit->nreported == 0)
    {
      h3result_echo(f, "   [No individual domains that satisfy reporting "
                       "thresholds (although complete target did)]\n");
      continue;
    }

    /* The domain table is 101 char wide. */
    h3result_echo(f,
                  " %3s   %6s %5s %9s %9s %7s %7s %2s %7s %7s %2s %7s %7s "
                  "%2s %4s",
                  "#", "score", "bias", "c-Evalue", "i-Evalue", "hmmfrom",
                  "hmm to", "  ", "alifrom", "ali to", "  ", "envfrom",
                  "env to", "  ", "acc");
    h3result_echo(f,
                  " %3s   %6s %5s %9s %9s %7s %7s %2s %7s %7s %2s %7s %7s "
                  "%2s %4s",
                  "---", "------", "-----", "---------", "---------", "-------",
                  "-------", "  ", "-------", "-------", "  ", "-------",
                  "-------", "  ", "----");

    unsigned dnum = 0;
    for (unsigned j = 0; j < hit->ndomains; j++)
    {
      struct domain const *dom = hit->domains + j;
      if (!dom->is_reported) continue;

      dnum++;
      fprintf(f, " %3u %c %6.1f %5.1f %9.2g %9.2g", dnum, included_symbol(dom),
              dom->bitscore, dombits(dom), evalue(dom->lnP, domZ),
              evalue(dom->lnP, Z));

      print_range(f, dom->ad.hmmfrom, dom->ad.hmmto, dom->ad.M);
      print_range(f, dom->ad.sqfrom, dom->ad.sqto, dom->ad.L);
      print_range(f, dom->ienv, dom->jenv, dom->ad.L);

      h3result_echo(f, " %4.2f", prob_ali_res(dom));
    }

    /* Alignment data for each reported domain in this reported
     * sequence. */
    h3result_echo(f, "\n  Alignments for each domain:");
    dnum = 0;

    for (unsigned j = 0; j < hit->ndomains; j++)
    {
      struct domain const *dom = hit->domains + j;
      if (!dom->is_reported) continue;

      dnum++;
      fprintf(f, "  == domain %d", dnum);
      fprintf(f, "  score: %.1f bits", dom->bitscore);
      h3result_echo(f, ";  conditional E-value: %.2g", evalue(dom->lnP, domZ));

      h3result_alidisplay_print(&dom->ad, f);
      h3result_echo(f, "%s", "");
    }
  }

  if (x->nreported == 0)
    h3result_echo(f, "\n   [No targets detected that satisfy reporting "
                     "thresholds]");
}

static unsigned max_acc_length(struct tophits const *th)
{
  unsigned max = 0;
  for (unsigned i = 0; i < th->nhits; i++)
    max = max(max, (unsigned)strlen(th->hits[i].acc));
  return max;
}

static unsigned qname_width(struct tophits const *th)
{
  unsigned width = 20;

  for (unsigned i = 0; i < th->nhits; i++)
  {
    struct hit *const hit = th->hits + i;
    for (unsigned j = 0; j < hit->ndomains; j++)
    {
      struct domain const *dom = hit->domains + j;
      width = max(width, (unsigned)strlen(dom->ad.sqname));
    }
  }
  return width;
}

struct header_width
{
  unsigned qname;
  unsigned qacc;
  unsigned tname;
  unsigned tacc;
};

static void print_targets_table_header(FILE *f, struct header_width w)
{
  h3result_echo(f, "#%*s %22s %22s %33s",
                w.tname + w.qname + w.tacc + w.qacc + 2, "",
                "--- full sequence ----", "--- best 1 domain ----",
                "--- domain number estimation ----");
  h3result_echo(f,
                "#%-*s %-*s %-*s %-*s %9s %6s %5s %9s %6s %5s %5s %3s %3s "
                "%3s %3s %3s %3s %3s %s",
                w.tname - 1, " target name", w.tacc, "accession", w.qname,
                "query name", w.qacc, "accession", "  E-value", " score",
                " bias", "  E-value", " score", " bias", "exp", "reg", "clu",
                " ov", "env", "dom", "rep", "inc", "description of target");
  h3result_echo(f,
                "#%*s %*s %*s %*s %9s %6s %5s %9s %6s %5s %5s %3s %3s %3s "
                "%3s %3s %3s %3s %s",
                w.tname - 1, "-------------------", w.tacc, "----------",
                w.qname, "--------------------", w.qacc, "----------",
                "---------", "------", "-----", "---------", "------", "-----",
                "---", "---", "---", "---", "---", "---", "---", "---",
                "---------------------");
}

void h3result_tophits_print_targets_table(char const *qacc,
                                          struct tophits const *th, FILE *f,
                                          bool show_header, double Z)
{
  struct header_width w = {qname_width(th), max(10U, (unsigned)strlen(qacc)),
                           max(20U, max_name_length(th)),
                           max(10U, max_acc_length(th))};

  if (show_header) print_targets_table_header(f, w);

  for (unsigned i = 0; i < th->nhits; i++)
  {
    struct hit *const hit = th->hits + i;
    if (!(hit->flags & p7_IS_REPORTED)) continue;

    struct domain const *dom = hit->domains + hit->best_domain;
    char const *qname = dom->ad.sqname;
    h3result_echo(f,
                  "%-*s %-*s %-*s %-*s %9.2g %6.1f %5.1f %9.2g %6.1f "
                  "%5.1f %5.1f %3d %3d %3d %3d %3d %3d %3d %s",
                  w.tname, hit->name, w.tacc, strdash(hit->acc), w.qname, qname,
                  w.qacc, strdash(qacc), evalue(hit->lnP, Z), hit->score,
                  unbiased_score(hit), evalue(dom->lnP, Z), dom->bitscore,
                  dombits(dom), hit->nexpected, hit->nregions, hit->nclustered,
                  hit->noverlaps, hit->nenvelopes, hit->ndomains,
                  hit->nreported, hit->nincluded, hit->desc);
  }
}

static void print_domains_table_header(struct header_width w, FILE *f)
{
  h3result_echo(f, "#%*s %22s %40s %11s %11s %11s",
                w.tname + w.qname - 1 + 15 + w.tacc + w.qacc, "",
                "--- full sequence ---",
                "-------------- this domain -------------", "hmm coord",
                "ali coord", "env coord");
  h3result_echo(f,
                "#%-*s %-*s %5s %-*s %-*s %5s %9s %6s %5s %3s %3s %9s %9s "
                "%6s %5s %5s %5s %5s %5s %5s %5s %4s %s",
                w.tname - 1, " target name", w.tacc, "accession", "tlen",
                w.qname, "query name", w.qacc, "accession", "qlen", "E-value",
                "score", "bias", "#", "of", "c-Evalue", "i-Evalue", "score",
                "bias", "from", "to", "from", "to", "from", "to", "acc",
                "description of target");
  h3result_echo(f,
                "#%*s %*s %5s %*s %*s %5s %9s %6s %5s %3s %3s %9s %9s %6s "
                "%5s %5s %5s %5s %5s %5s %5s %4s %s",
                w.tname - 1, "-------------------", w.tacc, "----------",
                "-----", w.qname, "--------------------", w.qacc, "----------",
                "-----", "---------", "------", "-----", "---", "---",
                "---------", "---------", "------", "-----", "-----", "-----",
                "-----", "-----", "-----", "-----", "----",
                "---------------------");
}

void h3result_tophits_print_domains_table(char const *qacc,
                                          struct tophits const *th, FILE *f,
                                          bool show_header, double Z,
                                          double domZ)
{
  struct header_width w = {20, 10, 20, 10};
  for (unsigned i = 0; i < th->nhits; i++)
  {
    struct hit *const hit = th->hits + i;
    struct domain const *dom = hit->domains + hit->best_domain;
    w.qname = max(w.qname, (unsigned)strlen(dom->ad.sqname));
  }

  w.tname = max(w.tname, max_name_length(th));
  w.qacc = max(w.qacc, (unsigned)strlen(qacc));
  w.tacc = max(w.tacc, max_acc_length(th));

  if (show_header) print_domains_table_header(w, f);

  for (unsigned i = 0; i < th->nhits; i++)
  {
    struct hit *const hit = th->hits + i;

    if (!(hit->flags & p7_IS_REPORTED)) continue;

    unsigned dnum = 0;
    for (unsigned j = 0; j < hit->ndomains; j++)
    {
      struct domain const *dom = hit->domains + j;
      if (!dom->is_reported) continue;
      dnum++;

      char const *qname = dom->ad.sqname;
      unsigned qlen = dom->ad.L;
      unsigned tlen = dom->ad.M;

      h3result_echo(
          f,
          "%-*s %-*s %5d %-*s %-*s %5d %9.2g %6.1f %5.1f %3d "
          "%3d %9.2g %9.2g %6.1f %5.1f %5u %5u %5u %5u %5lu %5lu %4.2f "
          "%s",
          w.tname, hit->name, w.tacc, strdash(hit->acc), tlen, w.qname, qname,
          w.qacc, strdash(qacc), qlen, evalue(hit->lnP, Z), hit->score,
          unbiased_score(hit), dnum, hit->nreported, evalue(dom->lnP, domZ),
          evalue(dom->lnP, Z), dom->bitscore, dombits(dom), dom->ad.hmmfrom,
          dom->ad.hmmto, dom->ad.sqfrom, dom->ad.sqto, dom->ienv, dom->jenv,
          prob_ali_res(dom), strdash(hit->desc));
    }
  }
}

char const *h3result_tophits_hit_name(struct tophits const *th, unsigned idx)
{
  return th->hits[idx].name;
}

char const *h3result_tophits_hit_acc(struct tophits const *th, unsigned idx)
{
  return th->hits[idx].acc;
}

double h3result_tophits_hit_evalue_ln(struct tophits const *th, unsigned idx,
                                      double Z)
{
  return evalue_ln(th->hits[idx].lnP, Z);
}
