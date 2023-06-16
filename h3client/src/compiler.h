#ifndef COMPILER_H
#define COMPILER_H

#include <assert.h>

#define STATIC_ASSERT(test_for_true)                                           \
  static_assert((test_for_true), "(" #test_for_true ") failed")

#endif
