#ifndef AMSG_H
#define AMSG_H

#include <stddef.h>

struct amsg;
struct nng_iov;
struct nng_stream;

struct amsg *h3client_asend(struct nng_stream *, int len, struct nng_iov *,
                            void (*callb)(void *), void *arg, long deadline);
struct amsg *h3client_arecv(struct nng_stream *, size_t len, void *data,
                            void (*callb)(void *), void *arg, long deadline);
void h3client_astart(struct amsg *);
void h3client_adel(struct amsg *);
void h3client_acancel(struct amsg *);
void h3client_astop(struct amsg *);
int h3client_aresult(struct amsg *);

#endif
