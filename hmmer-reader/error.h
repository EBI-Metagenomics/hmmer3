#ifndef ERROR_H
#define ERROR_H

struct hmr_tok;

int hmr_error(int rc, char *dst, char const *msg);
int hmr_error_io(char *dst, int errnum);
int hmr_error_parse(struct hmr_tok *tok, char const *msg);

#endif
