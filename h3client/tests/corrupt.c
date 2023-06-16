#include "fs.h"
#include "h3client/h3client.h"
#include "helper.h"
#include "hope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51371
static char const cmd[] = "--hmmdb 1 --acc --cut_ga --hmmdb_ranges 0..4";

static void test_corrupt(void);

int main(void)
{
  atexit(h3client_fini);
  test_corrupt();
  return hope_status();
}

static void test_corrupt(void)
{
  struct h3client_dialer *d = h3client_dialer_new("127.0.0.1", PORT);
  notnull(d);
  eq(h3client_dialer_dial(d, h3client_deadline(1000)), 0);

  struct h3client_stream *s = h3client_dialer_stream(d);

  struct h3client_result *result = h3client_result_new();
  notnull(result);

  long deadline = h3client_deadline(1000 * 5);
  eq(h3client_stream_put(s, cmd, corrupt->name, corrupt->seq, deadline), 0);

  h3client_stream_wait(s);
  eq(h3client_stream_pop(s, result), 0);
  eq(h3client_result_errnum(result), 7);
  eq(h3client_result_errstr(result), "Error parsing FASTA sequence");

  h3client_result_del(result);
  h3client_stream_del(s);
  h3client_dialer_del(d);
}
