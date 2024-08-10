#ifndef HMR_HMR_H
#define HMR_HMR_H

#include "hmr_aux.h"
#include "hmr_error.h"
#include "hmr_prof.h"
#include "hmr_state.h"
#include "hmr_tok.h"
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

void hmr_init(struct hmr *hmr, FILE *restrict fp);
int hmr_next_prof(struct hmr *hmr, struct hmr_prof *prof);
int hmr_next_node(struct hmr *hmr, struct hmr_prof *prof);
void hmr_clear_error(struct hmr *hmr);
int hmr_count_profiles(char const *filepath);

#endif
