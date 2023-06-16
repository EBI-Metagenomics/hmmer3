#ifndef HOPE_H
#define HOPE_H

#include "hope_support.h"

#define close(actual, desired)                                                 \
  CLOSE2((double)(actual), (double)(desired), __HOPE_REL_TOL(actual), 0.0)

#define CLOSE2(actual, desired, rel_tol, abs_tol)                              \
  __hope_close2((double)(actual), (double)(desired), (double)(rel_tol),        \
                (double)(abs_tol), __FILE__, __LINE__)

#define eq(actual, desired) __hope_eq((actual), (desired), __FILE__, __LINE__)

#define isnull(a) __hope_isnull((a) != NULL, #a, __FILE__, __LINE__)

#define notnull(a) __hope_notnull((a) == NULL, #a, __FILE__, __LINE__)

#define cond(a) __hope_cond(#a, !(a), __FILE__, __LINE__)

inline static int hope_status(void) { return __hope_errors; }

#endif
