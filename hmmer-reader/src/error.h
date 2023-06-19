#ifndef ERROR_H
#define ERROR_H

struct hmr_tok;

int hmr_err(int rc, char *dst, char const *msg);
int hmr_eio(char *dst, int errnum);
int hmr_eparse(struct hmr_tok *tok, char const *msg);

#endif
