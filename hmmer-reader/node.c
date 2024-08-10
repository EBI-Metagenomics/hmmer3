#include "node.h"
#include "hmmer_reader/node.h"
#include <math.h>

void hmr_node_dump(struct hmr_node const *node, FILE *restrict fp)
{
    if (node->idx == 0)
        fprintf(fp, "COMPO");
    else
        fprintf(fp, "%*u", 5, node->idx);
    for (unsigned i = 0; i < node->symbols_size; ++i)
        fprintf(fp, " %.5f", node->compo[i]);
    fprintf(fp, "\n");

    fprintf(fp, "     ");
    for (unsigned i = 0; i < node->symbols_size; ++i)
        fprintf(fp, " %.5f", node->insert[i]);
    fprintf(fp, "\n");

    fprintf(fp, "     ");
    for (unsigned i = 0; i < HMR_TRANS_SIZE; ++i)
        fprintf(fp, " %.5f", node->trans[i]);
    fprintf(fp, "\n");
}

void hmr_node_init(struct hmr_node *node)
{
    node->symbols_size = 0;
    node->idx = 0;
    for (unsigned i = 0; i < HMR_SYMBOLS_MAX; ++i)
    {
        node->compo[i] = NAN;
        node->insert[i] = NAN;
    }
    for (unsigned i = 0; i < HMR_TRANS_SIZE; ++i)
        node->trans[i] = NAN;
}
