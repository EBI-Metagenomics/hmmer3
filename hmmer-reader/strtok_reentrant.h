#ifndef STRTOK_REENTRANT
#define STRTOK_REENTRANT

#include <string.h>

static char *strtok_reentrant(char *restrict s, const char *restrict sep,
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
