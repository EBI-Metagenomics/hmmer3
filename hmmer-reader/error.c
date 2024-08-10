#define _POSIX_C_SOURCE 200112L

#include "error.h"
#include "aux.h"
#include "hmmer_reader/aux.h"
#include "hmmer_reader/error.h"
#include "hmmer_reader/prof.h"
#include "hmmer_reader/tok.h"
#include <assert.h>
#include <stdarg.h>
#include <string.h>

static char prefix[][20] = {
    "", "", "", "IO error:", "Runtime error:", "Parse error:"};

#define unused(x) ((void)(x))

int hmr_err(int rc, char *dst, char const *msg)
{
    int n = snprintf(dst, HMR_ERROR_SIZE, "%s %s", prefix[rc], msg);
    assert(0 < n && n < HMR_ERROR_SIZE);
    unused(n);
    return rc;
}

int hmr_eio(char *dst, int errnum)
{
    int rc = strerror_r(errnum, dst, HMR_ERROR_SIZE);
    assert(!rc);
    unused(rc);
    return HMR_EIO;
}

int hmr_eparse(struct hmr_tok *tok, char const *msg)
{
    int n = snprintf(tok->error, HMR_ERROR_SIZE, "%s %s: line %d",
                     prefix[HMR_EPARSE], msg, tok->line.number);
    assert(0 < n && n < HMR_ERROR_SIZE);
    unused(n);
    return HMR_EPARSE;
}
