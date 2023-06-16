#include "fs.h"
#include "h3client/h3client.h"
#include "helper.h"
#include "hope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51371
static char const cmd[] = "--hmmdb 1 --acc --cut_ga --hmmdb_ranges 0..4";

static void test_multi(void);

int main(void)
{
  atexit(h3client_fini);
  test_multi();
  return hope_status();
}

static void test_multi(void)
{
  struct h3client_dialer *d = h3client_dialer_new("127.0.0.1", PORT);
  notnull(d);
  eq(h3client_dialer_dial(d, h3client_deadline(1000)), 0);

  struct h3client_stream *s = h3client_dialer_stream(d);

  struct h3client_result *result = h3client_result_new();
  notnull(result);

  long deadline = h3client_deadline(1000 * 5);
  for (size_t i = 0; i < array_size(ross); ++i)
  {
    eq(h3client_stream_put(s, cmd, ross[i].name, ross[i].seq, deadline), 0);
  }

  for (size_t i = 0; i < array_size(ross); ++i)
  {
    h3client_stream_wait(s);
    eq(h3client_stream_pop(s, result), 0);
    eq(h3client_result_errnum(result), 0);
    eq(h3client_result_nhits(result), ross[i].expect.nhits);
    if (h3client_result_nhits(result) > 0)
    {
      double lev = h3client_result_hit_evalue_ln(result, 0);
      close(lev, ross[i].expect.ln_evalue);
    }
  }

  h3client_result_del(result);
  h3client_stream_del(s);
  h3client_dialer_del(d);
}
