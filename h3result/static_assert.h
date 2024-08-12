#ifndef STATIC_ASSERT2_H
#define STATIC_ASSERT2_H

#include <assert.h>

#define h3result_static_assert(test_for_true)                                  \
  static_assert((test_for_true), "(" #test_for_true ") failed")

#endif
