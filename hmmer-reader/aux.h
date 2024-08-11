#ifndef AUX_H
#define AUX_H

#include "hmr.h"
#include <stddef.h>

static void hmr_aux_init(struct hmr_aux *aux)
{
    aux->prof.begin = NULL;
    aux->prof.pos = NULL;
    aux->prof.end = NULL;
    aux->node.begin = NULL;
    aux->node.pos = NULL;
    aux->node.end = NULL;
    aux->idx = 0;
}

#endif
