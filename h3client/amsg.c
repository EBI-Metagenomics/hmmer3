#include "amsg.h"
#include "nng/nng.h"
#include "timeout.h"
#include <stdint.h>
#include <string.h>

struct amsg
{
  int len;
  struct nng_iov iov[8];
  struct nng_aio *upper_aio;
  struct nng_aio *lower_aio;
  struct nng_stream *s;
  void (*submit)(struct nng_stream *, struct nng_aio *);
};

static struct amsg *alloc(nng_stream *s,
                          void (*submit)(nng_stream *, nng_aio *), int len,
                          struct nng_iov *, void (*callb)(void *), void *arg,
                          long deadline);
static void destroy(struct amsg *x);
static void start(struct amsg *x);
static void callback(void *arg);

struct amsg *h3client_asend(struct nng_stream *s, int len, struct nng_iov *iov,
                            void (*callb)(void *), void *arg, long deadline)
{
  return alloc(s, &nng_stream_send, len, iov, callb, arg, deadline);
}

struct amsg *h3client_arecv(struct nng_stream *s, size_t len, void *data,
                            void (*callb)(void *), void *arg, long deadline)
{
  struct nng_iov iov = {.iov_buf = data, .iov_len = len};
  return alloc(s, &nng_stream_recv, 1, &iov, callb, arg, deadline);
}

void h3client_astart(struct amsg *x) { start(x); }

void h3client_adel(struct amsg *x) { destroy(x); }

void h3client_acancel(struct amsg *x) { nng_aio_cancel(x->upper_aio); }

void h3client_astop(struct amsg *x) { nng_aio_stop(x->upper_aio); }

int h3client_aresult(struct amsg *x) { return nng_aio_result(x->upper_aio); }

static int consume(int len, struct nng_iov *iov, size_t amount);

static struct amsg *alloc(nng_stream *s,
                          void (*submit)(nng_stream *, nng_aio *), int len,
                          struct nng_iov *iov, void (*callb)(void *), void *arg,
                          long deadline)
{
  struct amsg *x;

  if ((x = nng_alloc(sizeof(*x))) == NULL) return NULL;
  if (nng_aio_alloc(&x->upper_aio, callb, arg) != 0)
  {
    destroy(x);
    return NULL;
  }
  if (nng_aio_alloc(&x->lower_aio, &callback, x) != 0)
  {
    destroy(x);
    return NULL;
  }

  int duration = h3client_timeout(deadline);
  nng_aio_set_timeout(x->upper_aio, duration);
  nng_aio_set_timeout(x->lower_aio, duration);

  nng_aio_begin(x->upper_aio);

  x->s = s;
  x->len = len;
  memcpy(x->iov, iov, len * sizeof(*iov));
  x->submit = submit;

  return x;
}

static void destroy(struct amsg *x)
{
  if (!x) return;
  if (x->lower_aio)
  {
    nng_aio_free(x->lower_aio);
    x->lower_aio = NULL;
  }
  if (x->upper_aio)
  {
    nng_aio_free(x->upper_aio);
    x->upper_aio = NULL;
  }
  nng_free(x, sizeof(*x));
}

static void start(struct amsg *x)
{
  nng_aio_set_iov(x->lower_aio, x->len, x->iov);
  x->submit(x->s, x->lower_aio);
}

static void callback(void *arg)
{
  struct amsg *x = arg;
  int rv;
  size_t n;

  rv = nng_aio_result(x->lower_aio);
  if (rv != 0)
  {
    nng_aio_finish(x->upper_aio, rv);
    return;
  }
  n = nng_aio_count(x->lower_aio);

  if (!consume(x->len, x->iov, n))
  {
    nng_aio_finish(x->upper_aio, 0);
    return;
  }

  start(x);
}

static int consume(int len, struct nng_iov *iov, size_t amount)
{
  int rem = len;
  for (int i = 0; i < len; ++i)
  {
    size_t sz = amount < iov->iov_len ? amount : iov->iov_len;
    iov->iov_len -= sz;
    iov->iov_buf += sz;
    amount -= sz;
    rem -= iov->iov_len == 0;
    ++iov;
  }
  return rem;
}
