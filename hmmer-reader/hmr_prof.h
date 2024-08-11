#ifndef HMR_PROF_H
#define HMR_PROF_H

#include "hmr_node.h"

enum
{
    HMR_HEADER_MAX = 64,
    HMR_NAME_MAX = 64,
    HMR_ACC_MAX = 64,
    HMR_DESC_MAX = 128,
    HMR_LENG_MAX = 8,
    HMR_ALPH_MAX = 12,
    HMR_GA_MAX = 32,
    HMR_BUFF_MAX = 128,
};

struct hmr_prof
{
    char header[HMR_HEADER_MAX];
    struct
    {
        char name[HMR_NAME_MAX];
        char acc[HMR_ACC_MAX];
        char desc[HMR_DESC_MAX];
        char leng[HMR_LENG_MAX];
        char alph[HMR_ALPH_MAX];
        char ga[HMR_GA_MAX];
    } meta;
    char buff[HMR_BUFF_MAX];
    unsigned symbols_size;
    char symbols[HMR_SYMBOLS_MAX];

    struct hmr_node node;
    char *error;
};

#endif
