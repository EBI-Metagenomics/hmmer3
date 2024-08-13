#include "hmmd_alidisplay.h"
#include "compiler.h"
#include "defer_return.h"
#include "h3c_errno.h"
#include "utils.h"
#include "zc.h"
#include <stdlib.h>
#include <string.h>

void h3client_hmmd_alidisplay_init(struct hmmd_alidisplay *ali)
{
  memset(ali, 0, sizeof(*ali));
}

void h3client_hmmd_alidisplay_cleanup(struct hmmd_alidisplay *ali)
{
  free(ali->mem);
  ali->mem = NULL;
  h3client_hmmd_alidisplay_init(ali);
}

static_assert(sizeof(int) == sizeof(uint32_t), "HMMER3 undefired requirement");
#define SER_BASE_SIZE ((5 * sizeof(int)) + (3 * sizeof(int64_t)) + 1)

#define RFLINE_PRESENT (1 << 0)
#define MMLINE_PRESENT (1 << 1)
#define CSLINE_PRESENT (1 << 2)
#define PPLINE_PRESENT (1 << 3)
#define ASEQ_PRESENT (1 << 4)
#define NTSEQ_PRESENT (1 << 5)

static bool parse_strings(struct hmmd_alidisplay *ali, size_t size, char **mem)
{
  unsigned nstrings = 0;
  nstrings += !!(ali->presence & RFLINE_PRESENT);
  nstrings += !!(ali->presence & MMLINE_PRESENT);
  nstrings += !!(ali->presence & CSLINE_PRESENT);
  nstrings += 2;
  nstrings += !!(ali->presence & ASEQ_PRESENT);
  nstrings += !!(ali->presence & NTSEQ_PRESENT);
  nstrings += !!(ali->presence & PPLINE_PRESENT);
  nstrings += 6;

  if (!h3client_expect_n_strings(size, *mem, nstrings)) return false;

  ali->rfline = ali->presence & RFLINE_PRESENT ? h3client_strskip(mem) : 0;
  ali->mmline = ali->presence & MMLINE_PRESENT ? h3client_strskip(mem) : 0;
  ali->csline = ali->presence & CSLINE_PRESENT ? h3client_strskip(mem) : 0;
  ali->model = h3client_strskip(mem);
  ali->mline = h3client_strskip(mem);
  ali->aseq = ali->presence & ASEQ_PRESENT ? h3client_strskip(mem) : 0;
  ali->ntseq = ali->presence & NTSEQ_PRESENT ? h3client_strskip(mem) : 0;
  ali->ppline = ali->presence & PPLINE_PRESENT ? h3client_strskip(mem) : 0;

  ali->hmmname = h3client_strskip(mem);
  ali->hmmacc = h3client_strskip(mem);
  ali->hmmdesc = h3client_strskip(mem);
  ali->sqname = h3client_strskip(mem);
  ali->sqacc = h3client_strskip(mem);
  ali->sqdesc = h3client_strskip(mem);

  return true;
}

int h3client_hmmd_alidisplay_parse(struct hmmd_alidisplay *ali,
                                   unsigned char const **ptr,
                                   unsigned char const *end)
{
  int rc = 0;

  if (end < *ptr + sizeof(uint32_t)) defer_return(H3C_EPARSE);

  size_t obj_size = h3client_eatu32(ptr);
  if (obj_size <= SER_BASE_SIZE) defer_return(H3C_EPARSE);
  size_t memsize = (size_t)(obj_size - SER_BASE_SIZE);

  if (!(ali->mem = zc_reallocf(ali->mem, memsize))) defer_return(H3C_ENOMEM);
  ali->memsize = memsize;

  if (end < *ptr + 4 * sizeof(uint32_t) + 3 * sizeof(uint64_t)) defer_return(H3C_EPARSE);
  ali->N = h3client_eatu32(ptr);
  ali->hmmfrom = h3client_eatu32(ptr);
  ali->hmmto = h3client_eatu32(ptr);
  ali->M = h3client_eatu32(ptr);
  ali->sqfrom = h3client_eatu64(ptr);
  ali->sqto = h3client_eatu64(ptr);
  ali->L = h3client_eatu64(ptr);

  if (end < *ptr + sizeof(uint8_t)) defer_return(H3C_EPARSE);
  ali->presence = h3client_eatu8(ptr);

  if (end < *ptr + memsize) defer_return(H3C_EPARSE);
  memcpy(ali->mem, *ptr, memsize);
  *ptr += memsize;

  char *mem = ali->mem;
  if (!parse_strings(ali, ali->memsize, &mem)) defer_return(H3C_EPARSE);

  return 0;

defer:
  h3client_hmmd_alidisplay_cleanup(ali);
  return rc;
}
