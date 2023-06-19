#ifndef FSM_H
#define FSM_H

#include "hmmer_reader/state.h"

struct hmr_aux;
struct hmr_prof;
struct hmr_tok;

void hmr_fsm_init(enum hmr_state *state);

enum hmr_state hmr_fsm_next(enum hmr_state state, struct hmr_tok *tok,
                            struct hmr_aux *aux, struct hmr_prof *prof);

char const *hmr_fsm_name(enum hmr_state state);

#endif
