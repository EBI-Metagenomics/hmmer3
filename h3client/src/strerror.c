#include "h3client/strerror.h"
#include "array_size.h"
#include "h3client/errno.h"
#include <stdio.h>

static char const *msg[] = {
    [H3CLIENT_EUNKNOWN] = "unknown error",
    [H3CLIENT_EPARSE] = "parse error",
    [H3CLIENT_EUNPACK] = "unpack failure",
    [H3CLIENT_EPACK] = "pack failure",
    [H3CLIENT_EUNEOF] = "unexpected end-of-file",
    [H3CLIENT_EOUTRANGE] = "out-of-range integer",
    [H3CLIENT_ETIMEDOUT] = "timed out",
    [H3CLIENT_ECANCELED] = "operation was canceled",
    [H3CLIENT_EADDRINVAL] = "invalid address",
    [H3CLIENT_ECLOSED] = "operation was aborted",
    [H3CLIENT_ECONNREFUSED] = "connection was refused",
    [H3CLIENT_ECONNRESET] = "connection was reset",
    [H3CLIENT_EINVAL] = "invalid value",
    [H3CLIENT_ENOMEM] = "insufficient free memory",
    [H3CLIENT_EPEERAUTH] = "authentication failure",
    [H3CLIENT_EPROTO] = "protocol error occurred",
    [H3CLIENT_EUNREACHABLE] = "address is not reachable",
    [H3CLIENT_ECONNSHUT] = "remote peer shutdown after sending data",
};

char const *h3client_strerror(int errno)
{
  if (errno > 0 && errno < (int)array_size(msg)) return msg[errno];

  static char unknown[32] = {0};
  snprintf(unknown, sizeof unknown, "unknown error #%d", errno);
  return unknown;
}
