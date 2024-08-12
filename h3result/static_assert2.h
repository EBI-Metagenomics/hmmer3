#ifndef STATIC_ASSERT2_H
#define STATIC_ASSERT2_H

#include <assert.h>

#ifdef _Static_assert
#define static_assert2(x) _Static_assert((x), "(" #x ") failed")
#else
#define static_assert2(x) static_assert((x), "(" #x ") failed")
#endif

#endif
