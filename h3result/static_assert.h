#ifndef H3RESULT_STATIC_ASSERT_H
#define H3RESULT_STATIC_ASSERT_H

#include <assert.h>

#define h3result_static_assert(test_for_true)                                  \
  static_assert((test_for_true), "(" #test_for_true ") failed")

#endif
