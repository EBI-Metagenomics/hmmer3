#ifndef H3RESULT_ECHO_H
#define H3RESULT_ECHO_H

#include <stdio.h>

void h3result_echo(FILE *, char const *, ...)
    __attribute__((format(printf, 2, 3)));

#endif
