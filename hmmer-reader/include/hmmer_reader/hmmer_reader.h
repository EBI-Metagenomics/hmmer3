#ifndef HMR_HMR_H
#define HMR_HMR_H

#include "hmmer_reader/aux.h"
#include "hmmer_reader/error.h"
#include "hmmer_reader/export.h"
#include "hmmer_reader/node.h"
#include "hmmer_reader/prof.h"
#include "hmmer_reader/rc.h"
#include "hmmer_reader/state.h"
#include "hmmer_reader/tok.h"
#include "hmmer_reader/trans.h"
#include <stdio.h>

struct hmr
{
    FILE *restrict fp;
    enum hmr_state state;
    struct hmr_tok tok;
    struct hmr_aux aux;
    char error[HMR_ERROR_SIZE];
};

#define HMR_DECLARE(name, fp)                                                  \
    struct hmr name;                                                           \
    hmr_init(&name, fp)

HMR_API void hmr_init(struct hmr *hmr, FILE *restrict fp);
HMR_API int hmr_next_prof(struct hmr *hmr, struct hmr_prof *prof);
HMR_API int hmr_next_node(struct hmr *hmr, struct hmr_prof *prof);
HMR_API void hmr_clear_error(struct hmr *hmr);
HMR_API int hmr_count_profiles(char const *filepath);

#endif
