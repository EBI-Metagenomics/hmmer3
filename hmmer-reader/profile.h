#ifndef PROFILE_H
#define PROFILE_H

#include "hmr.h"
#include <stdio.h>

struct hmr_position;
struct hmr_profile;
struct hmr_token;

int hmr_profile_next_node(struct hmr_profile *, FILE *restrict fp,
                          struct hmr_position *, enum hmr_state *,
                          struct hmr_token *);

int hmr_profile_next_profile(struct hmr_profile *, FILE *restrict fp,
                             struct hmr_position *, enum hmr_state *,
                             struct hmr_token *);

#endif
