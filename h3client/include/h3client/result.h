#ifndef H3CLIENT_RESULT_H
#define H3CLIENT_RESULT_H

#include "h3client/export.h"
#include <stdbool.h>
#include <stdio.h>

struct h3client_result;

H3CLIENT_API struct h3client_result *h3client_result_new(void);
H3CLIENT_API void h3client_result_del(struct h3client_result const *);

H3CLIENT_API int h3client_result_pack(struct h3client_result const *, FILE *);
H3CLIENT_API int h3client_result_unpack(struct h3client_result *, FILE *);

H3CLIENT_API int h3client_result_errnum(struct h3client_result const *);
H3CLIENT_API char const *h3client_result_errstr(struct h3client_result const *);

H3CLIENT_API void h3client_result_print_targets(struct h3client_result const *,
                                                FILE *);
H3CLIENT_API void h3client_result_print_domains(struct h3client_result const *,
                                                FILE *);

H3CLIENT_API void
h3client_result_print_targets_table(struct h3client_result const *, FILE *);
H3CLIENT_API void
h3client_result_print_domains_table(struct h3client_result const *, FILE *);

H3CLIENT_API unsigned h3client_result_nhits(struct h3client_result const *);
H3CLIENT_API char const *
h3client_result_hit_name(struct h3client_result const *, unsigned idx);
H3CLIENT_API char const *h3client_result_hit_acc(struct h3client_result const *,
                                                 unsigned idx);
H3CLIENT_API double
h3client_result_hit_evalue_ln(struct h3client_result const *, unsigned idx);

#endif
