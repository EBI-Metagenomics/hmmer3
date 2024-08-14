#include "nnge.h"
#include "h3c_errno.h"
#include "nng/nng.h"

int h3client_nnge(int c)
{
  if (c == NNG_ETIMEDOUT)    return H3C_ETIMEDOUT;
  if (c == NNG_ECANCELED)    return H3C_ECANCELED;
  if (c == NNG_EADDRINVAL)   return H3C_EADDRINVAL;
  if (c == NNG_ECLOSED)      return H3C_ECLOSED;
  if (c == NNG_ECONNREFUSED) return H3C_ECONNREFUSED;
  if (c == NNG_ECONNRESET)   return H3C_ECONNRESET;
  if (c == NNG_EINVAL)       return H3C_EINVAL;
  if (c == NNG_ENOMEM)       return H3C_ENOMEM;
  if (c == NNG_EPEERAUTH)    return H3C_EPEERAUTH;
  if (c == NNG_EPROTO)       return H3C_EPROTO;
  if (c == NNG_EUNREACHABLE) return H3C_EUNREACHABLE;
  if (c == NNG_ECONNSHUT)    return H3C_ECONNSHUT;
  return c ? H3C_EUNKNOWN : 0;
}
