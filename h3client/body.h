#ifndef BODY_H
#define BODY_H

#include <stddef.h>

struct body;
struct h3r;

struct body *body_new(void);
int          body_parse(struct body *, size_t size, unsigned char const buffer[size]);
int          body_store(struct body *, struct h3r *);
void         body_del(struct body const *);

#endif
