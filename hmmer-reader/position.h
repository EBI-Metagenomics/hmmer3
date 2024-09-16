#ifndef POSITION_H
#define POSITION_H

#include "hmmer_reader.h"
#include <stddef.h>

static void hmr_position_init(struct hmr_position *pos)
{
    pos->prof.begin = NULL;
    pos->prof.pos = NULL;
    pos->prof.end = NULL;
    pos->node.begin = NULL;
    pos->node.pos = NULL;
    pos->node.end = NULL;
    pos->idx = 0;
}

#endif
