#ifndef H3RESULT_H3RESULT_H
#define H3RESULT_H3RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "export.h"
#include "strerror.h"
#include <stdio.h>

struct h3result;

H3RESULT_API struct h3result *h3result_new(void);
H3RESULT_API void h3result_del(struct h3result const *);

H3RESULT_API int h3result_pack(struct h3result const *, FILE *);
H3RESULT_API int h3result_unpack(struct h3result *, FILE *);

H3RESULT_API int h3result_errnum(struct h3result const *);
H3RESULT_API char const *h3result_errstr(struct h3result const *);

H3RESULT_API void h3result_print_targets(struct h3result const *, FILE *);
H3RESULT_API void h3result_print_domains(struct h3result const *, FILE *);

H3RESULT_API void h3result_print_targets_table(struct h3result const *, FILE *);
H3RESULT_API void h3result_print_domains_table(struct h3result const *, FILE *);

H3RESULT_API unsigned h3result_nhits(struct h3result const *);
H3RESULT_API char const *h3result_hit_name(struct h3result const *,
                                           unsigned idx);
H3RESULT_API char const *h3result_hit_acc(struct h3result const *,
                                          unsigned idx);
H3RESULT_API double h3result_hit_evalue_ln(struct h3result const *,
                                           unsigned idx);

#ifdef __cplusplus
}
#endif

#endif
