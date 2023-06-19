#include "to.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static bool to_double(char const *str, double *val);

bool hmr_to_lprob(char const *str, double *val)
{
    bool ok = to_double(str, val);
    *val = -(*val);
    return ok;
}

bool hmr_to_uint(char const *str, unsigned *val)
{
    unsigned long v = strtoul(str, NULL, 10);
    *val = (unsigned)v;
    return (v <= UINT_MAX || !(v == 0 && !(str[0] == '0' && str[1] == '\0')));
}

static bool to_double(char const *str, double *val)
{
    if (str[0] == '*' && str[1] == '\0')
    {
        *val = INFINITY;
        return true;
    }
    char *ptr = NULL;
    *val = strtod(str, &ptr);

    return !((*val == 0.0 && str == ptr) || strchr(str, '\0') != ptr);
}
