#ifndef HMR_H
#define HMR_H

#include "hmr_prof.h"
#include "hmr_struct.h"
#include <stdio.h>

#define HMR_DECLARE(name, fp) struct hmr name; hmr_init(&name, fp)

void hmr_init(struct hmr *hmr, FILE *restrict fp);
int  hmr_next_prof(struct hmr *hmr, struct hmr_prof *prof);
int  hmr_next_node(struct hmr *hmr, struct hmr_prof *prof);
void hmr_clear_error(struct hmr *hmr);
int  hmr_count_profiles(char const *filepath);

#define HMR_PROF_DECLARE(name, hmr) struct hmr_prof name; hmr_prof_init(&name, (hmr))

void     hmr_prof_dump(struct hmr_prof const *prof, FILE *restrict fp);
void     hmr_prof_init(struct hmr_prof *prof, struct hmr *hmr);
unsigned hmr_prof_length(struct hmr_prof const *prof);

#endif
