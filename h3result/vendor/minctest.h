/*
 *
 * MINCTEST - Minimal C Test Library - 0.3.0
 *
 * Copyright (c) 2014-2021 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#ifndef MINCTEST_H
#define MINCTEST_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* How far apart can floats be before we consider them unequal. */
#ifndef LTEST_FLOAT_TOLERANCE
#define LTEST_FLOAT_TOLERANCE 0.001
#endif

/* Track the number of passes, fails. */
/* NB this is made for all tests to be in one file. */
static size_t ltests = 0;
static size_t lfails = 0;

/* Display the test results. */
#define lresults()                                                             \
  do                                                                           \
  {                                                                            \
    if (lfails == 0)                                                           \
    {                                                                          \
      printf("ALL TESTS PASSED (%zu/%zu)\n", ltests, ltests);                  \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      printf("SOME TESTS FAILED (%zu/%zu)\n", ltests - lfails, ltests);        \
    }                                                                          \
  } while (0)

/* Run a test. Name can be any string to print out, test is the function name to
 * call. */
#define lrun(name, test)                                                       \
  do                                                                           \
  {                                                                            \
    const size_t ts = ltests;                                                  \
    const size_t fs = lfails;                                                  \
    const clock_t start = clock();                                             \
    printf("\t%s:\n", name);                                                   \
    test();                                                                    \
    printf("\t -- pass: %-20zu fail: %-20zu time: %ldms\n",                    \
           (ltests - ts) - (lfails - fs), lfails - fs,                         \
           (long)((clock() - start) * 1000 / CLOCKS_PER_SEC));                 \
  } while (0)

/* Assert a true statement. */
#define _minctest_ok(test)                                                     \
  do                                                                           \
  {                                                                            \
    ++ltests;                                                                  \
    if (!(test))                                                               \
    {                                                                          \
      ++lfails;                                                                \
      printf("%s:%d error \n", __FILE__, __LINE__);                            \
    }                                                                          \
  } while (0)

#define _minctest_sprintf(buf, val)                                            \
  sprintf(buf,                                                                 \
          _Generic((val) + 0,                                                  \
          char *: "%s",                                                        \
          char const *: "%s",                                                  \
          signed char: "%d",                                                   \
          unsigned char: "%u",                                                 \
          short: "%d",                                                         \
          unsigned short: "%u",                                                \
          int: "%d",                                                           \
          long: "%ld",                                                         \
          long long: "%lld",                                                   \
          unsigned: "%u",                                                      \
          unsigned long: "%lu",                                                \
          unsigned long long: "%llu",                                          \
          float: "%.9g",                                                       \
          double: "%.17g",                                                     \
          long double: "%Lf",                                                  \
          default: _Generic((val - val), ptrdiff_t: "%p", default: "undef")),  \
          (val))

/* Prototype to assert equal. */
#define _minctest_eq_base(equality, a, b)                                      \
  do                                                                           \
  {                                                                            \
    ++ltests;                                                                  \
    if (!(equality))                                                           \
    {                                                                          \
      ++lfails;                                                                \
      char _bufa_mt[64] = {0};                                                 \
      char _bufb_mt[64] = {0};                                                 \
      _minctest_sprintf(_bufa_mt, a);                                          \
      _minctest_sprintf(_bufb_mt, b);                                          \
      printf("%s:%d (%s != %s)\n", __FILE__, __LINE__, _bufa_mt, _bufb_mt);    \
    }                                                                          \
  } while (0);

/* Assert two integers are equal. */
#define _minctest_eq(a, b)                                                     \
  ({                                                                           \
    __typeof__((a) + 0) _a = (a);                                              \
    __typeof__((b) + 0) _b = (b);                                              \
    _minctest_eq_base((_a) == (_b), _a, _b)                                    \
  })

/* Assert two floats are equal (Within LTEST_FLOAT_TOLERANCE). */
#define _minctest_close(a, b)                                                  \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _minctest_eq_base(fabs((double)(_a) - (double)(_b)) <=                     \
                              LTEST_FLOAT_TOLERANCE &&                         \
                          fabs((double)(_a) - (double)(_b)) ==                 \
                              fabs((double)(_a) - (double)(_b)),               \
                      (double)(_a), (double)(_b))                              \
  })

/* Assert two strings are equal. */
#define _minctest_cmp(a, b) _minctest_eq_base(strcmp(a, b) == 0, a, b)

#define eq(a, b) _minctest_eq(a, b)
#define close(a, b) _minctest_close(a, b)
#define ok(x) _minctest_ok(x)
#define cmp(a, b) _minctest_cmp(a, b)

#endif
