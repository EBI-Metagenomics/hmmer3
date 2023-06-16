#ifndef HMSG_H
#define HMSG_H

#include <stddef.h>

struct answer;
struct hmsg;
struct nng_stream;

struct hmsg *h3client_hrecv(struct nng_stream *, struct answer *,
                            void (*callb)(void *), void *arg, long deadline);
void h3client_hstart(struct hmsg *);
void h3client_hcancel(struct hmsg *);
void h3client_hstop(struct hmsg *);
void h3client_hdel(struct hmsg *);

int h3client_hresult(struct hmsg *);
struct answer *h3client_houtput(struct hmsg *);

#endif
