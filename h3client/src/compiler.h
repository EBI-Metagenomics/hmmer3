#ifndef COMPILER_H
#define COMPILER_H

#include <assert.h>

#ifndef static_assert
#define static_assert(expr, msg) _Static_assert(expr, msg)
#endif

#define STATIC_ASSERT(test_for_true)                                           \
  static_assert((test_for_true), "(" #test_for_true ") failed")

#endif
