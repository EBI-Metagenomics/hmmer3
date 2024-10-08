#ifndef FSM_H
#define FSM_H

#include "hmmer_reader.h"

struct hmr_position;
struct hmr_profile;
struct hmr_token;

void hmr_fsm_init(enum hmr_state *state);

enum hmr_state hmr_fsm_next(enum hmr_state state, struct hmr_token *tok,
                            struct hmr_position *pos, struct hmr_profile *prof);

char const *hmr_fsm_name(enum hmr_state state);

#endif
