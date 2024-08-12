#ifndef POSIX_H
#define POSIX_H

#include <string.h>

static char *posix_strtok_r(char *restrict s, const char *restrict sep,
                            char **restrict p)
{
    if (!s && !(s = *p)) return NULL;
    s += strspn(s, sep);
    if (!*s) return *p = 0;
    *p = s + strcspn(s, sep);
    if (**p)
        *(*p)++ = 0;
    else
        *p = 0;
    return s;
}

#endif
