#ifndef H3CLIENT_DIALER_H
#define H3CLIENT_DIALER_H

#include "h3client/export.h"

struct h3client_dialer;
struct h3client_stream;

H3CLIENT_API struct h3client_dialer *h3client_dialer_new(char const *ip,
                                                         int port);
H3CLIENT_API int h3client_dialer_dial(struct h3client_dialer *, long deadline);
H3CLIENT_API struct h3client_stream *
h3client_dialer_stream(struct h3client_dialer *x);
H3CLIENT_API void h3client_dialer_del(struct h3client_dialer *);

#endif
