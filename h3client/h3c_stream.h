#ifndef H3C_STREAM_H
#define H3C_STREAM_H

struct nng_stream;
struct h3client_stream;
struct h3c_result;

struct h3client_stream *           h3client_stream_new(struct nng_stream *);
int                                h3client_stream_put(struct  h3client_stream *, char const *args, char const *name, char const *seq, long deadline);
void                               h3client_stream_wait(struct h3client_stream *);
int                                h3client_stream_pop(struct  h3client_stream *, struct h3c_result *);
void                               h3client_stream_del(struct  h3client_stream *);

#endif
