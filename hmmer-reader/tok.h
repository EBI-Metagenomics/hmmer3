#ifndef TOK_H
#define TOK_H

#include <stdio.h>

struct hmr_tok;

void hmr_tok_init(struct hmr_tok *tok, char *error);
int hmr_tok_next(struct hmr_tok *tok, FILE *restrict fp);

#endif
