#ifndef PROF_H
#define PROF_H

#include "hmr.h"
#include <stdio.h>

struct hmr_position;
struct hmr_profile;
struct hmr_token;

int hmr_profile_next_node(struct hmr_profile *prof, FILE *restrict fp,
                          struct hmr_position *aux, enum hmr_state *state,
                          struct hmr_token *tok);

int hmr_profile_next_profile(struct hmr_profile *prof, FILE *restrict fp,
                             struct hmr_position *aux, enum hmr_state *state,
                             struct hmr_token *tok);

#endif
