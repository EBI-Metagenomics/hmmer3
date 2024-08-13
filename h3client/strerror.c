#include "h3client/strerror.h"
#include "array_size.h"
#include "h3c_errno.h"
#include <stdio.h>

static char const *msg[] = {
    [H3C_EUNKNOWN] = "unknown error",
    [H3C_EPARSE] = "parse error",
    [H3C_EUNPACK] = "unpack failure",
    [H3C_EPACK] = "pack failure",
    [H3C_EUNEOF] = "unexpected end-of-file",
    [H3C_EOUTRANGE] = "out-of-range integer",
    [H3C_ETIMEDOUT] = "timed out",
    [H3C_ECANCELED] = "operation was canceled",
    [H3C_EADDRINVAL] = "invalid address",
    [H3C_ECLOSED] = "operation was aborted",
    [H3C_ECONNREFUSED] = "connection was refused",
    [H3C_ECONNRESET] = "connection was reset",
    [H3C_EINVAL] = "invalid value",
    [H3C_ENOMEM] = "insufficient free memory",
    [H3C_EPEERAUTH] = "authentication failure",
    [H3C_EPROTO] = "protocol error occurred",
    [H3C_EUNREACHABLE] = "address is not reachable",
    [H3C_ECONNSHUT] = "remote peer shutdown after sending data",
};

char const *h3client_strerror(int errno)
{
  if (errno > 0 && errno < (int)array_size(msg)) return msg[errno];

  static char unknown[32] = {0};
  snprintf(unknown, sizeof unknown, "unknown error #%d", errno);
  return unknown;
}
