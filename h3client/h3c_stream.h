#ifndef H3C_STREAM_H
#define H3C_STREAM_H

struct h3client_stream;
struct h3client_result;

int h3client_stream_put(struct h3client_stream *, char const *args,
                        char const *name, char const *seq, long deadline);
void h3client_stream_wait(struct h3client_stream *);
int h3client_stream_pop(struct h3client_stream *, struct h3client_result *);
void h3client_stream_del(struct h3client_stream *);

#endif
