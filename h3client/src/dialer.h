#ifndef DIALER_H
#define DIALER_H

struct h3client_dialer;
struct h3client_stream;

struct h3client_dialer *h3client__dialer_new(char const *ip, int port);
int h3client__dialer_dial(struct h3client_dialer *, long deadline);
struct h3client_stream *h3client__dialer_stream(struct h3client_dialer *);
void h3client__dialer_del(struct h3client_dialer *);

#endif
