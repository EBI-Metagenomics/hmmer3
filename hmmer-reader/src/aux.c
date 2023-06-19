#include "aux.h"
#include "hmmer_reader/aux.h"
#include <stdlib.h>

void hmr_aux_init(struct hmr_aux *aux)
{
    aux->prof.begin = NULL;
    aux->prof.pos = NULL;
    aux->prof.end = NULL;
    aux->node.begin = NULL;
    aux->node.pos = NULL;
    aux->node.end = NULL;
    aux->idx = 0;
}
