#ifndef HMR_TOK_H
#define HMR_TOK_H

#include <stdbool.h>

enum hmr_tok_id
{
    HMR_TOK_NL,
    HMR_TOK_WORD,
    HMR_TOK_HMM,
    HMR_TOK_COMPO,
    HMR_TOK_SLASH,
    HMR_TOK_EOF,
};

enum
{
    HMR_TOK_LINE_MAX = 256,
};

struct hmr_tok
{
    enum hmr_tok_id id;
    char const *value;
    struct
    {
        char data[HMR_TOK_LINE_MAX];
        unsigned number;
        bool consumed;
        char *ctx;
    } line;
    char *error;
};

#endif
