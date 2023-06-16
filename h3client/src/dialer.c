#include "dialer.h"
#include "h3client/errno.h"
#include "h3client/stream.h"
#include "itoa.h"
#include "nng/nng.h"
#include "nnge.h"
#include "stream.h"
#include "timeout.h"
#include <stdlib.h>
#include <string.h>

struct h3client_dialer
{
  nng_stream_dialer *stream;
  nng_aio *aio;
  struct h3client_stream *h3client_stream;
};

struct h3client_dialer *h3client_dialer_new(char const *ip, int port)
{
  struct h3client_dialer *dialer = malloc(sizeof(*dialer));
  if (!dialer) return NULL;

  char uri[512] = "tcp://";
  strncat(uri, ip, sizeof(uri) - strlen(uri) - 27);
  strcat(uri, ":");
  h3client_itoa(uri + strlen(uri), port);

  dialer->stream = NULL;
  dialer->aio = NULL;
  dialer->h3client_stream = NULL;

  if (nng_stream_dialer_alloc(&dialer->stream, uri))
  {
    free(dialer);
    return NULL;
  }

  if (nng_aio_alloc(&dialer->aio, NULL, NULL))
  {
    nng_stream_dialer_close(dialer->stream);
    nng_stream_dialer_free(dialer->stream);
    free(dialer);
    return NULL;
  }

  return dialer;
}

int h3client_dialer_dial(struct h3client_dialer *x, long deadline)
{
  nng_aio_set_timeout(x->aio, h3client_timeout(deadline));
  nng_stream_dialer_dial(x->stream, x->aio);
  nng_aio_wait(x->aio);

  int rc = h3client_nnge(nng_aio_result(x->aio));
  if (rc) return rc;

  struct nng_stream *s = nng_aio_get_output(x->aio, 0);
  struct h3client_stream *stream = h3client_stream_new(s);
  if (!stream)
  {
    nng_stream_close(s);
    nng_stream_free(s);
    return H3CLIENT_ENOMEM;
  }
  x->h3client_stream = stream;
  return 0;
}

struct h3client_stream *h3client_dialer_stream(struct h3client_dialer *x)
{
  struct h3client_stream *s = x->h3client_stream;
  x->h3client_stream = NULL;
  return s;
}

void h3client_dialer_del(struct h3client_dialer *x)
{
  if (!x) return;

  if (x->h3client_stream) h3client_stream_del(x->h3client_stream);
  if (x->aio) nng_aio_free(x->aio);
  if (x->stream)
  {
    nng_stream_dialer_close(x->stream);
    nng_stream_dialer_free(x->stream);
  }

  free(x);
}
