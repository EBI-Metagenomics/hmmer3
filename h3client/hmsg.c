#include "hmsg.h"
#include "amsg.h"
#include "answer.h"
#include "h3c_errno.h"
#include "hmmd_hmmd.h"
#include "nng/nng.h"
#include "nnge.h"
#include "timeout.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum state
{
  STATUS,
  DATA,
};

struct hmsg
{
  enum state state;
  struct nng_stream *stream;
  struct nng_aio *aio;
  struct answer *ans;
  struct amsg *amsg0;
  struct amsg *amsg1;
  long deadline;
};

static void callback(void *arg)
{
  struct hmsg *x = arg;
  int rc = nng_aio_result(x->aio);
  if (rc != 0)
  {
    nng_aio_finish(x->aio, rc);
    return;
  }

  if (x->state == STATUS)
  {
    rc = h3client_aresult(x->amsg0);
    if (rc != 0)
    {
      nng_aio_finish(x->aio, rc);
      return;
    }
    struct hmmd_status const *status = h3client_answer_status_parse(x->ans);
    size_t size = status->msg_size;

    if ((rc = h3client_answer_setup_size(x->ans, size)))
    {
      nng_aio_finish(x->aio, NNG_ENOMEM);
      return;
    }

    void *data = h3client_answer_data(x->ans);
    if (!(x->amsg1 =
              h3client_arecv(x->stream, size, data, &callback, x, x->deadline)))
    {
      nng_aio_finish(x->aio, NNG_ENOMEM);
      return;
    }
    h3client_astart(x->amsg1);
    x->state = DATA;
  }
  else if (x->state == DATA)
  {
    rc = h3client_aresult(x->amsg1);
    if (rc != 0)
    {
      nng_aio_finish(x->aio, rc);
      return;
    }
    if (h3client_answer_status(x->ans)->status)
    {
      if ((rc = h3client_answer_parse_error(x->ans)))
      {
        nng_aio_finish(x->aio, NNG_EINVAL);
        return;
      }
    }
    else
    {
      if ((rc = h3client_answer_parse(x->ans)))
      {
        nng_aio_finish(x->aio, NNG_EINVAL);
        return;
      }
    }
    nng_aio_finish(x->aio, 0);
  }
}

struct hmsg *h3client_hrecv(struct nng_stream *stream, struct answer *ans,
                            void (*callb)(void *), void *arg, long deadline)
{
  struct hmsg *x = malloc(sizeof(*x));
  if (!x) return NULL;

  x->state = STATUS;
  x->stream = NULL;
  x->aio = NULL;
  x->ans = NULL;
  x->amsg0 = NULL;
  x->amsg1 = NULL;
  x->deadline = deadline;

  x->stream = stream;
  if (nng_aio_alloc(&x->aio, callb, arg))
  {
    h3client_hdel(x);
    return NULL;
  }
  nng_aio_set_timeout(x->aio, h3client_timeout(deadline));
  nng_aio_begin(x->aio);

  x->ans = ans;

  void *data = h3client_answer_status_data(ans);
  size_t size = h3client_answer_status_size();
  char *b = data;
  for (int i = 0; i < (int)size; ++i)
    b[i] = 0;
  if (!(x->amsg0 = h3client_arecv(stream, size, data, &callback, x, deadline)))
  {
    h3client_hdel(x);
    return NULL;
  }

  return x;
}

void h3client_hstart(struct hmsg *x) { h3client_astart(x->amsg0); }

void h3client_hcancel(struct hmsg *x) { nng_aio_cancel(x->aio); }

void h3client_hstop(struct hmsg *x) { nng_aio_stop(x->aio); }

void h3client_hdel(struct hmsg *x)
{
  if (!x) return;
  if (x->amsg1) h3client_adel(x->amsg1);
  if (x->amsg0) h3client_adel(x->amsg0);
  if (x->aio) nng_aio_free(x->aio);
  free(x);
}

int h3client_hresult(struct hmsg *x) { return nng_aio_result(x->aio); }

struct answer *h3client_houtput(struct hmsg *x) { return x->ans; }
