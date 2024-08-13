#ifndef H3CLIENT_ANSWER_H
#define H3CLIENT_ANSWER_H

#include <stddef.h>

struct answer;
struct h3r;

struct answer *h3client_answer_new(void);
void h3client_answer_del(struct answer const *ans);

unsigned char *h3client_answer_status_data(struct answer *ans);
size_t h3client_answer_status_size(void);
struct hmmd_status const *h3client_answer_status_parse(struct answer *ans);
struct hmmd_status const *h3client_answer_status(struct answer const *ans);

int h3client_answer_setup_size(struct answer *ans, size_t size);
unsigned char *h3client_answer_data(struct answer *ans);
int h3client_answer_parse(struct answer *ans);
int h3client_answer_parse_error(struct answer *ans);
int h3client_answer_copy(struct answer *ans, struct h3r *);

#endif
