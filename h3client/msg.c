#include "msg.h"
#include "amsg.h"
#include "answer.h"
#include "array_size.h"
#include "h3c_errno.h"
#include "hmsg.h"
#include "nng/nng.h"
#include "nng/supplemental/util/platform.h"
#include "nnge.h"
#include "timeout.h"
#include <stdlib.h>
#include <string.h>

struct msg *h3client_msg_new(struct nng_stream *stream)
{
  struct msg *x = malloc(sizeof(*x));
  if (!x) return x;

  x->stream = stream;
  x->state = 0;
  x->deadline = 0;
  x->aio = NULL;
  x->ans = NULL;
  x->send_amsg = NULL;
  x->recv_hmsg = NULL;
  x->mtx = NULL;
  cco_node_init(&x->node);

  if (nng_aio_alloc(&x->aio, NULL, NULL))
  {
    free(x);
    return NULL;
  }

  if (!(x->ans = h3client_answer_new()))
  {
    nng_aio_free(x->aio);
    free(x);
    return NULL;
  }

  if (nng_mtx_alloc(&x->mtx))
  {
    h3client_answer_del(x->ans);
    nng_aio_free(x->aio);
    free(x);
    return NULL;
  }

  return x;
}

static void callback(void *arg);

int h3client_msg_start(struct msg *x, char const *args, char const *name,
                       char const *seq, long deadline)
{
  struct nng_iov iov[] = {
      {.iov_buf = "@", .iov_len = 1},
      {.iov_buf = (void *)args, .iov_len = strlen(args)},
      {.iov_buf = "\n>", .iov_len = 2},
      {.iov_buf = (void *)name, .iov_len = strlen(name)},
      {.iov_buf = "\n", .iov_len = 1},
      {.iov_buf = (void *)seq, .iov_len = strlen(seq)},
      {.iov_buf = "\n//", .iov_len = 3},
  };
  x->state = SEND;
  x->deadline = deadline;

  nng_aio_set_timeout(x->aio, h3client_timeout(deadline));
  nng_aio_begin(x->aio);

  if (!(x->send_amsg = h3client_asend(x->stream, array_size(iov), iov,
                                      &callback, x, deadline)))
    return H3C_ENOMEM;

  h3client_astart(x->send_amsg);
  return 0;
}

void h3client_msg_wait(struct msg *x) { nng_aio_wait(x->aio); }

int h3client_msg_result(struct msg *x)
{
  return h3client_nnge(nng_aio_result(x->aio));
}

void h3client_msg_cancel(struct msg *x)
{
  nng_mtx_lock(x->mtx);
  if (x->recv_hmsg) h3client_hcancel(x->recv_hmsg);
  nng_mtx_unlock(x->mtx);
  h3client_acancel(x->send_amsg);
  nng_aio_cancel(x->aio);
}

void h3client_msg_stop(struct msg *x)
{
  nng_mtx_lock(x->mtx);
  if (x->recv_hmsg) h3client_hstop(x->recv_hmsg);
  nng_mtx_unlock(x->mtx);
  h3client_astop(x->send_amsg);
  nng_aio_stop(x->aio);
}

void h3client_msg_del(struct msg const *x)
{
  if (!x) return;
  if (x->ans) h3client_answer_del(x->ans);
  if (x->send_amsg) h3client_adel(x->send_amsg);
  nng_mtx_lock(x->mtx);
  if (x->recv_hmsg) h3client_hdel(x->recv_hmsg);
  nng_mtx_unlock(x->mtx);
  if (x->mtx) nng_mtx_free(x->mtx);
  if (x->aio) nng_aio_free(x->aio);
  free((void *)x);
}

struct answer *h3client_msg_answer(struct msg *x) { return x->ans; }

static void callback(void *arg)
{
  struct msg *x = arg;

  if (x->state == SEND)
  {
    int rc = h3client_aresult(x->send_amsg);
    if (rc)
    {
      nng_aio_finish(x->aio, rc);
      return;
    }

    nng_mtx_lock(x->mtx);
    x->recv_hmsg = h3client_hrecv(x->stream, x->ans, &callback, x, x->deadline);
    nng_mtx_unlock(x->mtx);
    if (!x->recv_hmsg)
    {
      nng_aio_finish(x->aio, H3C_ENOMEM);
      return;
    }

    x->state = RECV;
    h3client_hstart(x->recv_hmsg);
  }
  else if (x->state == RECV)
  {
    nng_aio_finish(x->aio, h3client_hresult(x->recv_hmsg));
  }
}
