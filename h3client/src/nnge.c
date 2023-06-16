#include "nnge.h"
#include "h3client/errno.h"
#include "nng/nng.h"

int h3client_nnge(int c)
{
  if (c == NNG_ETIMEDOUT) return H3CLIENT_ETIMEDOUT;
  if (c == NNG_ECANCELED) return H3CLIENT_ECANCELED;
  if (c == NNG_EADDRINVAL) return H3CLIENT_EADDRINVAL;
  if (c == NNG_ECLOSED) return H3CLIENT_ECLOSED;
  if (c == NNG_ECONNREFUSED) return H3CLIENT_ECONNREFUSED;
  if (c == NNG_ECONNRESET) return H3CLIENT_ECONNRESET;
  if (c == NNG_EINVAL) return H3CLIENT_EINVAL;
  if (c == NNG_ENOMEM) return H3CLIENT_ENOMEM;
  if (c == NNG_EPEERAUTH) return H3CLIENT_EPEERAUTH;
  if (c == NNG_EPROTO) return H3CLIENT_EPROTO;
  if (c == NNG_EUNREACHABLE) return H3CLIENT_EUNREACHABLE;
  if (c == NNG_ECONNSHUT) return H3CLIENT_ECONNSHUT;
  return c ? H3CLIENT_EUNKNOWN : 0;
}
