#ifndef NODE_H
#define NODE_H

#include "hmr.h"
#include <math.h>

static void hmr_node_init(struct hmr_node *node)
{
    node->symbols_size = 0;
    node->idx = 0;
    for (unsigned i = 0; i < HMR_SYMBOLS_MAX; ++i)
    {
        node->compo[i] = (double)NAN;
        node->insert[i] = (double)NAN;
    }
    for (unsigned i = 0; i < HMR_TRANS_SIZE; ++i)
        node->trans[i] = (double)NAN;
}

#endif
