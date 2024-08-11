#ifndef HMR_STRUCT_H
#define HMR_STRUCT_H

#include "hmr_aux.h"
#include "hmr_tok.h"
#include <stdio.h>

enum
{
    HMR_ERROR_SIZE = 128,
};

enum
{
    HMR_OK,
    HMR_END,
    HMR_EOF,
    HMR_EIO,
    HMR_EUSAGE,
    HMR_EPARSE,
};

enum hmr_state
{
    HMR_FSM_BEGIN,
    HMR_FSM_HEADER,
    HMR_FSM_NAME,
    HMR_FSM_CONTENT,
    HMR_FSM_SYMBOL,
    HMR_FSM_ARROW,
    HMR_FSM_COMPO,
    HMR_FSM_INSERT,
    HMR_FSM_MATCH,
    HMR_FSM_TRANS,
    HMR_FSM_PAUSE,
    HMR_FSM_SLASHED,
    HMR_FSM_END,
    HMR_FSM_ERROR
};

struct hmr
{
    FILE *restrict fp;
    enum hmr_state state;
    struct hmr_tok tok;
    struct hmr_aux aux;
    char error[HMR_ERROR_SIZE];
};

#endif
