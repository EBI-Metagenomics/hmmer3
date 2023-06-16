#ifndef H3CLIENT_ERRNO_H
#define H3CLIENT_ERRNO_H

#include "h3client/export.h"

// clang-format off
enum h3client_errno
{
    H3CLIENT_EUNKNOWN     = 1,
    H3CLIENT_EPARSE       = 2,
    H3CLIENT_EUNPACK      = 3,
    H3CLIENT_EPACK        = 4,
    H3CLIENT_EUNEOF       = 5,
    H3CLIENT_EOUTRANGE    = 6,
    H3CLIENT_ETIMEDOUT    = 7,
    H3CLIENT_ECANCELED    = 8,
    H3CLIENT_EADDRINVAL   = 9,
    H3CLIENT_ECLOSED      = 10,
    H3CLIENT_ECONNREFUSED = 11,
    H3CLIENT_ECONNRESET   = 12,
    H3CLIENT_EINVAL       = 13,
    H3CLIENT_ENOMEM       = 14,
    H3CLIENT_EPEERAUTH    = 15,
    H3CLIENT_EPROTO       = 16,
    H3CLIENT_EUNREACHABLE = 17,
    H3CLIENT_ECONNSHUT    = 18,
};
// clang-format on

#endif
