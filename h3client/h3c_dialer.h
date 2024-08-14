#ifndef H3C_DIALER_H
#define H3C_DIALER_H

struct h3client_dialer;
struct h3client_stream;

struct h3client_dialer *h3client_dialer_new(char const *ip, int port);
int                     h3client_dialer_dial(struct h3client_dialer *, long deadline);
struct h3client_stream *h3client_dialer_stream(struct h3client_dialer *x);
void                    h3client_dialer_del(struct h3client_dialer *);

#endif
