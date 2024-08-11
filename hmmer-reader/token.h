#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

struct hmr_token;

void hmr_token_init(struct hmr_token *tok, char *error);
int hmr_token_next(struct hmr_token *tok, FILE *restrict fp);

#endif
