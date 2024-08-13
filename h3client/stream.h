#ifndef STREAM_H
#define STREAM_H

struct h3client_stream;
struct nng_stream;

struct h3client_stream *h3client_stream_new(struct nng_stream *stream);

#endif
