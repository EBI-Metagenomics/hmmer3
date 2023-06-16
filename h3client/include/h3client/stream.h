#ifndef H3CLIENT_STREAM_H
#define H3CLIENT_STREAM_H

#include "h3client/export.h"

struct h3client_stream;
struct h3client_result;

H3CLIENT_API int h3client_stream_put(struct h3client_stream *, char const *args,
                                     char const *name, char const *seq,
                                     long deadline);
H3CLIENT_API void h3client_stream_wait(struct h3client_stream *);
H3CLIENT_API int h3client_stream_pop(struct h3client_stream *,
                                     struct h3client_result *);
H3CLIENT_API void h3client_stream_del(struct h3client_stream *);

#endif
