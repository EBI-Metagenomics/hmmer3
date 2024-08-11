#ifndef ERROR_H
#define ERROR_H

struct hmr_token;

int hmr_error(int rc, char *dst, char const *msg);
int hmr_error_io(char *dst, int errnum);
int hmr_error_parse(struct hmr_token *tok, char const *msg);

#endif
