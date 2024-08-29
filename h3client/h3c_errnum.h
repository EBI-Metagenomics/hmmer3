#ifndef H3C_ERRNUM_H
#define H3C_ERRNUM_H

enum h3client_errnum
{
    H3C_EUNKNOWN     = 1,
    H3C_EPARSE       = 2,
    H3C_EUNPACK      = 3,
    H3C_EPACK        = 4,
    H3C_EUNEOF       = 5,
    H3C_EOUTRANGE    = 6,
    H3C_ETIMEDOUT    = 7,
    H3C_ECANCELED    = 8,
    H3C_EADDRINVAL   = 9,
    H3C_ECLOSED      = 10,
    H3C_ECONNREFUSED = 11,
    H3C_ECONNRESET   = 12,
    H3C_EINVAL       = 13,
    H3C_ENOMEM       = 14,
    H3C_EPEERAUTH    = 15,
    H3C_EPROTO       = 16,
    H3C_EUNREACHABLE = 17,
    H3C_ECONNSHUT    = 18,
    H3C_EDISCARD     = 19,
    H3C_EWARMUPCODE  = 20,
    H3C_EH3DAEMON    = 21,
    H3C_ERECV        = 22,
    H3C_ESEND        = 23,
    H3C_EH3RESULT    = 24,
};

#endif
