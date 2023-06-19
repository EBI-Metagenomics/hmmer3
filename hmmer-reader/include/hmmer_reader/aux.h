#ifndef HMR_AUX_H
#define HMR_AUX_H

struct hmr_aux
{
    union
    {
        struct
        {
            char *begin;
            char *pos;
            char *end;
        } prof;

        struct
        {
            double *begin;
            double *pos;
            double *end;
        } node;
    };
    unsigned idx;
};

#endif
