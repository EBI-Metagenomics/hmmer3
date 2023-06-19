#ifndef HOPE_SUPPORT_H
#define HOPE_SUPPORT_H

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int __hope_errors = 0;

static inline int __hope_close(double actual, double desired, double rel_tol,
                               double abs_tol);

static inline void __hope_print_context(const char *file, int line)
{
    fprintf(stderr, "\nAssertion error at %s:%d\n", file, line);
}

static inline void __hope_print_newline(void)
{
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define __HOPE_REL_TOL(x) _Generic((x), float : 5e-05, double : 1e-09)

static inline void __hope_close2(double actual, double desired, double rel_tol,
                                 double abs_tol, char const *file, int line)
{
    double a = actual;
    double d = desired;
    if (__hope_close(a, d, rel_tol, abs_tol))
    {
        __hope_print_context(file, line);
        fprintf(stderr, " Items are not close:\n");
        fprintf(stderr, "  ACTUAL : %.11f\n", (double)a);
        fprintf(stderr, "  DESIRED: %.11f\n", (double)d);
        __hope_print_newline();
        ++__hope_errors;
    }
}

#define __MAKE_EQ(S, T, F)                                                     \
    static inline void __hope_eq_##S(T a, T d, char const *file, int line)     \
    {                                                                          \
        if (!(a == d))                                                         \
        {                                                                      \
            __hope_print_context(file, line);                                  \
            fprintf(stderr, " Items are not equal:\n");                        \
            fprintf(stderr, "  ACTUAL : %" F "\n", a);                         \
            fprintf(stderr, "  DESIRED: %" F "\n", d);                         \
            __hope_print_newline();                                            \
            ++__hope_errors;                                                   \
        }                                                                      \
    }

__MAKE_EQ(hhu, unsigned char, "hhu")
__MAKE_EQ(hu, unsigned short, "hu")
__MAKE_EQ(u, unsigned int, "u")
__MAKE_EQ(lu, unsigned long, "lu")
__MAKE_EQ(llu, unsigned long long, "llu")

__MAKE_EQ(hhd, signed char, "hhd")
__MAKE_EQ(hd, short, "hd")
__MAKE_EQ(d, int, "d")
__MAKE_EQ(ld, long, "ld")
__MAKE_EQ(lld, long long, "lld")

#ifndef _WIN32
#define __hope_eq(actual, desired, file, line)                                 \
    _Generic((actual),                                                         \
             unsigned char: __hope_eq_hhu,                                     \
             unsigned short: __hope_eq_hu,                                     \
             unsigned int: __hope_eq_u,                                        \
             unsigned long: __hope_eq_lu,                                      \
             unsigned long long: __hope_eq_llu,                                \
             signed char: __hope_eq_hhd,                                       \
             short: __hope_eq_hd,                                              \
             int: __hope_eq_d,                                                 \
             long: __hope_eq_ld,                                               \
             long long: __hope_eq_lld,                                         \
             char: __hope_eq_char,                                             \
             char *: __hope_eq_str,                                            \
             char const *: __hope_eq_str,\
             FILE *: __hope_eq_file)((actual), (desired), file, line)
#else
/* Bug: https://is.gd/DWxyJO */
#define __hope_eq(actual, desired, file, line)                                 \
    _Generic((actual),                                                         \
             unsigned short: __hope_eq_hu,                                     \
             unsigned int: __hope_eq_u,                                        \
             unsigned long: __hope_eq_lu,                                      \
             unsigned long long: __hope_eq_llu,                                \
             short: __hope_eq_hd,                                              \
             int: __hope_eq_d,                                                 \
             long: __hope_eq_ld,                                               \
             long long: __hope_eq_lld,                                         \
             char: __hope_eq_char,                                             \
             char *: __hope_eq_str,                                            \
             char const *: __hope_eq_str,\
             FILE *: __hope_eq_file)((actual), (desired), file, line)
#endif

static inline void __hope_isnull(int cond, char const *expr, char const *file,
                                 int line)
{
    if (cond)
    {
        __hope_print_context(file, line);
        fprintf(stderr, " Address should be NULL:\n");
        fprintf(stderr, "  EXPRESSION: %s\n", expr);
        __hope_print_newline();
        ++__hope_errors;
    }
}

static inline void __hope_notnull(int cond, char const *expr, char const *file,
                                  int line)
{
    if (cond)
    {
        __hope_print_context(file, line);
        fprintf(stderr, " Address should not be NULL:\n");
        fprintf(stderr, "  EXPRESSION: %s\n", expr);
        __hope_print_newline();
        ++__hope_errors;
    }
}

static inline void __hope_eq_char(char a, char d, char const *file, int line)
{
    if (a != d)
    {
        __hope_print_context(file, line);
        fprintf(stderr, " Chars are not equal:\n");
        fprintf(stderr, "  ACTUAL : %c\n", a);
        fprintf(stderr, "  DESIRED: %c\n", d);
        __hope_print_newline();
        ++__hope_errors;
    }
}

static inline void __hope_eq_str(char const *a, char const *d, char const *file,
                                 int line)
{
    if (strcmp(a, d) != 0)
    {
        __hope_print_context(file, line);
        fprintf(stderr, " Strings are not equal:\n");
        fprintf(stderr, "  ACTUAL : %s\n", a);
        fprintf(stderr, "  DESIRED: %s\n", d);
        __hope_print_newline();
        ++__hope_errors;
    }
}

static inline void __hope_eq_file(FILE *restrict a, FILE *restrict d,
                                  char const *file, int line)
{
    char abuff[128];
    char dbuff[128];
    while (!feof(a) && !feof(d))
    {
        if (!fgets(abuff, sizeof abuff, a) && ferror(a)) goto io_error;
        if (!fgets(dbuff, sizeof dbuff, d) && ferror(d)) goto io_error;

        if (feof(a) != feof(d))
        {
            __hope_print_context(file, line);
            fprintf(stderr, " Files are not equal:\n");
            fprintf(stderr, "  End-of-files did not occurr simultaneously\n");
            __hope_print_newline();
            ++__hope_errors;
            return;
        }
        if (strcmp(abuff, dbuff) != 0)
        {
            __hope_print_context(file, line);
            fprintf(stderr, " Files are not equal:\n");
            fprintf(stderr, "  ACTUAL : %s\n", abuff);
            fprintf(stderr, "  DESIRED: %s\n", dbuff);
            __hope_print_newline();
            ++__hope_errors;
            return;
        }
    }
    return;

io_error:
    __hope_print_context(file, line);
    perror("failed to fgets");
    __hope_print_newline();
    ++__hope_errors;
    return;
}

static inline void __hope_cond(char const *expr, int cond, char const *file,
                               int line)
{
    if (cond)
    {
        __hope_print_context(file, line);
        fprintf(stderr, " Condition evaluates to false:\n");
        fprintf(stderr, "  EXPRESSION: %s\n", expr);
        __hope_print_newline();
        ++__hope_errors;
    }
}

static inline int __hope_close(double actual, double desired, double rel_tol,
                               double abs_tol)
{
    /* This implementation is basically a copy of the `math.isclose`
     * implementation of the Python library plus returning 0 in case
     * both values are NaN.
     */
    if (actual == desired)
    {
        /* short circuit exact equality -- needed to catch two infinities of
         * the same sign. And perhaps speeds things up a bit sometimes.
         */
        return 0;
    }

    if (isnan(actual) && isnan(desired))
    {
        return 0;
    }

    /* This catches the case of two infinities of opposite sign, or
     * one infinity and one finite number. Two infinities of opposite
     * sign would otherwise have an infinite relative tolerance.
     * Two infinities of the same sign are caught by the equality check
     * above.
     */

    if (isinf(actual) || isinf(desired))
    {
        return 1;
    }

    /* now do the regular computation
     * this is essentially the "weak" test from the Boost library
     */

    double diff = fabs(desired - actual);

    return !(((diff <= fabs(rel_tol * desired)) ||
              (diff <= fabs(rel_tol * actual))) ||
             (diff <= abs_tol));
}

#endif
