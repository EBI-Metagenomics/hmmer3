#include "aye.h"
#include "h3c_deadline.h"
#include "h3c_dialer.h"
#include "h3c_fini.h"
#include "h3c_result.h"
#include "h3c_stream.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>

#define PORT 51371

static char const cmd[] = "--hmmdb 1 --acc --cut_ga --hmmdb_ranges 0..4";

static void test_corrupt(void)
{
  struct h3client_dialer *d = h3client_dialer_new("127.0.0.1", PORT);
  aye(d);
  aye(!h3client_dialer_dial(d, h3client_deadline(1000)));

  struct h3client_stream *s = h3client_dialer_stream(d);

  struct h3c_result *result = h3c_result_new();
  aye(result);

  long deadline = h3client_deadline(1000 * 5);
  aye(!h3client_stream_put(s, cmd, corrupt->name, corrupt->seq, deadline));

  h3client_stream_wait(s);
  aye(!h3client_stream_pop(s, result));
  aye(result->errnum == 7);
  aye(!strcmp(result->errstr, "Error parsing FASTA sequence"));

  h3c_result_del(result);
  h3client_stream_del(s);
  h3client_dialer_del(d);
}

int main(void)
{
  aye_begin();
  atexit(h3client_fini);
  test_corrupt();
  return aye_end();
}
