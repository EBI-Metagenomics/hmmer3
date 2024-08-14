#ifndef H3C_RESULT_H
#define H3C_RESULT_H

#include "h3r_result.h"

struct h3c_result
{
  struct h3r *content;
  int errnum;
  char errstr[1024];
};

struct h3c_result *h3c_result_new(void);
void               h3c_result_del(struct h3c_result const *);

#endif
