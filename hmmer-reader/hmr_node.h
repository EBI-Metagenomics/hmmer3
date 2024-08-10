#ifndef HMR_NODE_H
#define HMR_NODE_H

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#ifndef static_assert
#define static_assert(expr, msg) _Static_assert(expr, msg)
#endif

enum
{
    HMR_SYMBOLS_MAX = 32,
    HMR_TRANS_SIZE = 7,
};

struct hmr_node
{
    unsigned symbols_size;
    unsigned idx;
    union
    {
        double compo[HMR_SYMBOLS_MAX];
        double match[HMR_SYMBOLS_MAX];
    };
    double insert[HMR_SYMBOLS_MAX];
    double trans[HMR_TRANS_SIZE];
    struct
    {
        unsigned map;
        union
        {
            struct
            {
                char cons;
                char rf;
                char mm;
                char cs;
            };
            char buf[4];
        };
    } excess;
};

#define HMR_NODE_MAP_NULL UINT_MAX

#define HMR_OFFSET(field) offsetof(struct hmr_node, excess.field)
static_assert(HMR_OFFSET(rf) - HMR_OFFSET(cons) == 1, "must be packed");
static_assert(HMR_OFFSET(mm) - HMR_OFFSET(rf) == 1, "must be packed");
static_assert(HMR_OFFSET(cs) - HMR_OFFSET(mm) == 1, "must be packed");
static_assert(HMR_OFFSET(buf) - HMR_OFFSET(cons) == 0, "must be packed");
#undef HMR_OFFSET

void hmr_node_dump(struct hmr_node const *node, FILE *restrict fp);

#endif
