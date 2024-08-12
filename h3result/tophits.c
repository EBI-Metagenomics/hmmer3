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

static void unset(struct tophits *x)
{
  x->nhits = 0;
  x->hits = NULL;
  x->nreported = 0;
  x->nincluded = 0;
  x->is_sorted_by_sortkey = false;
  x->is_sorted_by_seqidx = false;
}

void h3r_tophits_init(struct tophits *x) { unset(x); }

static int grow(struct tophits *x, unsigned nhits)
{
  size_t sz = nhits * sizeof(*x->hits);
  struct hit *hits = realloc(x->hits, sz);
  if (!hits) return H3RESULT_ENOMEM;
  x->hits = hits;

  for (unsigned i = x->nhits; i < nhits; ++i)
  {
    int rc = 0;
    if ((rc = h3r_hit_init(x->hits + i))) return rc;
    ++x->nhits;
  }

  return 0;
}

static void shrink(struct tophits *x, unsigned nhits)
{
  for (unsigned i = nhits; i < x->nhits; ++i)
    h3r_hit_cleanup(x->hits + i);

  x->nhits = nhits;
}

int h3r_tophits_setup(struct tophits *x, unsigned nhits)
{
  if (x->nhits < nhits) return grow(x, nhits);
  shrink(x, nhits);
  return 0;
}

void h3r_tophits_cleanup(struct tophits *x)
{
  for (unsigned i = 0; i < x->nhits; ++i)
    h3r_hit_cleanup(x->hits + i);
  free(x->hits);
  unset(x);
}

int h3r_tophits_pack(struct tophits const *th, struct lio_writer *f)
{
  if (write_array(f, 5)) return H3RESULT_EPACK;

  if (write_map(f, 1)) return H3RESULT_EPACK;
  if (write_cstring(f, "hits")) return H3RESULT_EPACK;
  if (write_array(f, th->nhits)) return H3RESULT_EPACK;

  for (unsigned i = 0; i < th->nhits; ++i)
  {
    int rc = h3r_hit_pack(th->hits + i, f);
    if (rc) return rc;
  }

  if (write_int(f, th->nreported)) return H3RESULT_EPACK;
  if (write_int(f, th->nincluded)) return H3RESULT_EPACK;
  if (write_bool(f, th->is_sorted_by_sortkey)) return H3RESULT_EPACK;
  if (write_bool(f, th->is_sorted_by_seqidx)) return H3RESULT_EPACK;

  return 0;
}

int h3r_tophits_unpack(struct tophits *th, struct lio_reader *f)
{

  if (!expect_array(f, 5)) return H3RESULT_EUNPACK;

  if (!expect_map(f, 1)) return H3RESULT_EUNPACK;
  if (!expect_key(f, "hits")) return H3RESULT_EUNPACK;

  unsigned size = 0;
  if (read_array(f, &size)) return H3RESULT_EUNPACK;

  int rc = 0;
  if ((rc = h3r_tophits_setup(th, size))) return H3RESULT_EUNPACK;

  for (unsigned i = 0; i < th->nhits; ++i)
  {
    if ((rc = h3r_hit_unpack(th->hits + i, f))) return H3RESULT_EUNPACK;
  }

  if (read_int(f, &th->nreported)) return H3RESULT_EUNPACK;
  if (read_int(f, &th->nincluded)) return H3RESULT_EUNPACK;
  if (read_bool(f, &th->is_sorted_by_sortkey)) return H3RESULT_EUNPACK;
  if (read_bool(f, &th->is_sorted_by_seqidx)) return H3RESULT_EUNPACK;

  return 0;
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

static inline char const *show_name(struct hit const *x)
{
  return (x->acc != 0 && x->acc[0] != '\0') ? x->acc : x->name;
}

static inline char const *strdash(char const *str)
{
  static char const dash[] = "-";
  return strlen(str) == 0 ? dash : str;
}

static inline char newness(struct hit const *x)
{
  char symbol = ' ';
  if (x->flags & p7_IS_NEW)
    symbol = '+';
  else if (x->flags & p7_IS_DROPPED)
    symbol = '-';
  return symbol;
}

#define CONST_LOG2R 1.44269504088896341

static inline double dombits(struct domain const *dom)
{
  return CONST_LOG2R * dom->dombias;
}

static inline float unbiased_score(struct hit const *x)
{
  return x->pre_score - x->score;
}

static inline double evalue(double lnP, double Z) { return exp(lnP) * Z; }
static inline double logevalue(double lnP, double Z) { return lnP + log(Z); }

int h3r_tophits_print_targets(struct tophits const *x, FILE *fp, double Z)
{
  unsigned namew = max(8U, max_shown_length(x));
  unsigned descw = max(32U, h3result_zero_clip(120 - namew - 61));

  if (echo(fp, "Scores for complete sequence (score includes all domains):"))                       return H3RESULT_EPRINT;
  if (echo(fp, "  %22s  %22s  %8s", " --- full sequence ---",  " --- best 1 domain ---", "-#dom-")) return H3RESULT_EPRINT;

  if (echo(fp, "  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s", "E-value",
           " score", " bias", "E-value", " score", " bias", "  exp", "N", namew,
           "Model", "Description")) return H3RESULT_EPRINT;

  if (echo(fp, "  %9s %6s %5s  %9s %6s %5s  %5s %2s  %-*s %s", "-------",
           "------", "-----", "-------", "------", "-----", " ----", "--",
           namew, "--------", "-----------")) return H3RESULT_EPRINT;

  bool printed_incthresh = false;
  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;
    if (!(hit->flags & p7_IS_REPORTED)) continue;

    struct domain const *dom = hit->domains + hit->best_domain;

    if (!(hit->flags & p7_IS_INCLUDED) && !printed_incthresh)
    {
      if (echo(fp, "  ------ inclusion threshold ------")) return H3RESULT_EPRINT;
      printed_incthresh = true;
    }

    if (echo(fp,
             "%c %9.2g %6.1f %5.1f  %9.2g %6.1f %5.1f  %5.1f %2d  %-*s  "
             "%-.*s",
             newness(hit), evalue(hit->lnP, Z), hit->score, unbiased_score(hit),
             evalue(dom->lnP, Z), dom->bitscore, dombits(dom), hit->nexpected,
             hit->nreported, namew, show_name(hit), descw, hit->desc)) return H3RESULT_EPRINT;
  }

  if (x->nreported == 0)
  {
    if (echo(fp, "\n   [No hits detected that satisfy reporting thresholds]")) return H3RESULT_EPRINT;
  }

  return 0;
}

#define p7_HITFLAGS_DEFAULT 0
#define p7_IS_INCLUDED (1 << 0)
#define p7_IS_REPORTED (1 << 1)
#define p7_IS_NEW (1 << 2)
#define p7_IS_DROPPED (1 << 3)
#define p7_IS_DUPLICATE (1 << 4)

static inline int echo_range(FILE *f, unsigned from, unsigned to,
                             unsigned length)
{
  return echo2(f, " %7u %7u %c%c", from, to, from == 1 ? '[' : '.',
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

int h3r_tophits_print_domains(struct tophits const *x, FILE *f, double Z,
                               double domZ)
{
  if (echo(f, "Domain annotation for each model (and alignments):")) return H3RESULT_EPRINT;

  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;
    if (!(hit->flags & p7_IS_REPORTED)) continue;

    char const *name = show_name(hit);
    unsigned namew = (unsigned)strlen(name);
    unsigned descw = max(32U, h3result_zero_clip(120 - namew - 5));

    if (echo(f, ">> %s  %-.*s", name, descw, hit->desc)) return H3RESULT_EPRINT;

    if (hit->nreported == 0)
    {
      echo(f, "   [No individual domains that satisfy reporting thresholds (although complete target did)]\n");
      continue;
    }

    /* The domain table is 101 char wide. */
    if (echo(f,
             " %3s   %6s %5s %9s %9s %7s %7s %2s %7s %7s %2s %7s %7s "
             "%2s %4s",
             "#", "score", "bias", "c-Evalue", "i-Evalue", "hmmfrom", "hmm to",
             "  ", "alifrom", "ali to", "  ", "envfrom", "env to", "  ", "acc")) return H3RESULT_EPRINT;
    if (echo(f,
             " %3s   %6s %5s %9s %9s %7s %7s %2s %7s %7s %2s %7s %7s "
             "%2s %4s",
             "---", "------", "-----", "---------", "---------", "-------",
             "-------", "  ", "-------", "-------", "  ", "-------", "-------",
             "  ", "----"))                                                      return H3RESULT_EPRINT;

    unsigned dnum = 0;
    for (unsigned j = 0; j < hit->ndomains; j++)
    {
      struct domain const *dom = hit->domains + j;
      if (!dom->is_reported) continue;

      dnum++;
      if (echo2(f, " %3u %c %6.1f %5.1f %9.2g %9.2g", dnum,
                included_symbol(dom), dom->bitscore, dombits(dom),
                evalue(dom->lnP, domZ), evalue(dom->lnP, Z)))
        return H3RESULT_EPRINT;

      if (echo_range(f, dom->ad.hmmfrom, dom->ad.hmmto, dom->ad.M)) return H3RESULT_EUNPACK;
      if (echo_range(f, dom->ad.sqfrom, dom->ad.sqto, dom->ad.L)) return H3RESULT_EUNPACK;
      if (echo_range(f, dom->ienv, dom->jenv, dom->ad.L)) return H3RESULT_EUNPACK;

      if (echo(f, " %4.2f", prob_ali_res(dom))) return H3RESULT_EPRINT;
    }

    /* Alignment data for each reported domain in this reported
     * sequence. */
    if (echo(f, "\n  Alignments for each domain:")) return H3RESULT_EPRINT;
    dnum = 0;

    for (unsigned j = 0; j < hit->ndomains; j++)
    {
      struct domain const *dom = hit->domains + j;
      if (!dom->is_reported) continue;

      dnum++;
      if (echo2(f, "  == domain %d", dnum))                                return H3RESULT_EPRINT;
      if (echo2(f, "  score: %.1f bits", dom->bitscore))                   return H3RESULT_EPRINT;
      if (echo(f, ";  conditional E-value: %.2g", evalue(dom->lnP, domZ))) return H3RESULT_EPRINT;

      h3r_alidisplay_print(&dom->ad, f);
      if (echo(f, "%s", "")) return H3RESULT_EPRINT;
    }
  }

  if (x->nreported == 0)
  {
    if (echo(f, "\n   [No targets detected that satisfy reporting thresholds]")) return H3RESULT_EPRINT;
  }
  return 0;
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
  echo(f, "#%*s %22s %22s %33s", w.tname + w.qname + w.tacc + w.qacc + 2, "",
       "--- full sequence ----", "--- best 1 domain ----",
       "--- domain number estimation ----");
  echo(f,
       "#%-*s %-*s %-*s %-*s %9s %6s %5s %9s %6s %5s %5s %3s %3s "
       "%3s %3s %3s %3s %3s %s",
       w.tname - 1, " target name", w.tacc, "accession", w.qname, "query name",
       w.qacc, "accession", "  E-value", " score", " bias", "  E-value",
       " score", " bias", "exp", "reg", "clu", " ov", "env", "dom", "rep",
       "inc", "description of target");
  echo(f,
       "#%*s %*s %*s %*s %9s %6s %5s %9s %6s %5s %5s %3s %3s %3s "
       "%3s %3s %3s %3s %s",
       w.tname - 1, "-------------------", w.tacc, "----------", w.qname,
       "--------------------", w.qacc, "----------", "---------", "------",
       "-----", "---------", "------", "-----", "---", "---", "---", "---",
       "---", "---", "---", "---", "---------------------");
}

int h3r_tophits_print_targets_table(char const *qacc, struct tophits const *x,
                                     FILE *f, bool show_header, double Z)
{
  struct header_width w = {qname_width(x), max(10U, (unsigned)strlen(qacc)),
                           max(20U, max_name_length(x)),
                           max(10U, max_acc_length(x))};

  if (show_header) print_targets_table_header(f, w);

  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;
    if (!(hit->flags & p7_IS_REPORTED)) continue;

    struct domain const *dom = hit->domains + hit->best_domain;
    char const *qname = dom->ad.sqname;
    echo(f,
         "%-*s %-*s %-*s %-*s %9.2g %6.1f %5.1f %9.2g %6.1f "
         "%5.1f %5.1f %3d %3d %3d %3d %3d %3d %3d %s",
         w.tname, hit->name, w.tacc, strdash(hit->acc), w.qname, qname, w.qacc,
         strdash(qacc), evalue(hit->lnP, Z), hit->score, unbiased_score(hit),
         evalue(dom->lnP, Z), dom->bitscore, dombits(dom), hit->nexpected,
         hit->nregions, hit->nclustered, hit->noverlaps, hit->nenvelopes,
         hit->ndomains, hit->nreported, hit->nincluded, hit->desc);
  }
  return 0;
}

static void print_domains_table_header(struct header_width w, FILE *f)
{
  echo(f, "#%*s %22s %40s %11s %11s %11s",
       w.tname + w.qname - 1 + 15 + w.tacc + w.qacc, "",
       "--- full sequence ---", "-------------- this domain -------------",
       "hmm coord", "ali coord", "env coord");
  echo(f,
       "#%-*s %-*s %5s %-*s %-*s %5s %9s %6s %5s %3s %3s %9s %9s "
       "%6s %5s %5s %5s %5s %5s %5s %5s %4s %s",
       w.tname - 1, " target name", w.tacc, "accession", "tlen", w.qname,
       "query name", w.qacc, "accession", "qlen", "E-value", "score", "bias",
       "#", "of", "c-Evalue", "i-Evalue", "score", "bias", "from", "to", "from",
       "to", "from", "to", "acc", "description of target");
  echo(f,
       "#%*s %*s %5s %*s %*s %5s %9s %6s %5s %3s %3s %9s %9s %6s "
       "%5s %5s %5s %5s %5s %5s %5s %4s %s",
       w.tname - 1, "-------------------", w.tacc, "----------", "-----",
       w.qname, "--------------------", w.qacc, "----------", "-----",
       "---------", "------", "-----", "---", "---", "---------", "---------",
       "------", "-----", "-----", "-----", "-----", "-----", "-----", "-----",
       "----", "---------------------");
}

int h3r_tophits_print_domains_table(char const *qacc, struct tophits const *x,
                                     FILE *f, bool show_header, double Z,
                                     double domZ)
{
  struct header_width w = {20, 10, 20, 10};
  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;
    struct domain const *dom = hit->domains + hit->best_domain;
    w.qname = max(w.qname, (unsigned)strlen(dom->ad.sqname));
  }

  w.tname = max(w.tname, max_name_length(x));
  w.qacc = max(w.qacc, (unsigned)strlen(qacc));
  w.tacc = max(w.tacc, max_acc_length(x));

  if (show_header) print_domains_table_header(w, f);

  for (unsigned i = 0; i < x->nhits; i++)
  {
    struct hit *const hit = x->hits + i;

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

      echo(f,
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
  return 0;
}

char const *h3r_tophits_hit_name(struct tophits const *x, unsigned idx)
{
  return x->hits[idx].name;
}

char const *h3r_tophits_hit_acc(struct tophits const *x, unsigned idx)
{
  return x->hits[idx].acc;
}

double h3r_tophits_hit_logevalue(struct tophits const *x, unsigned idx,
                                 double Z)
{
  return logevalue(x->hits[idx].lnP, Z);
}
