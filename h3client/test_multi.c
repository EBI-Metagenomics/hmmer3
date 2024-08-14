#include "aye.h"
#include "h3c_deadline.h"
#include "h3c_dialer.h"
#include "h3c_fini.h"
#include "h3c_result.h"
#include "h3c_stream.h"
#include "h3r_result.h"
#include "helper.h"
#include <math.h>
#include <stdlib.h>

#define PORT 51371

static char const cmd[] = "--hmmdb 1 --acc --cut_ga --hmmdb_ranges 0..4";

static inline int near(double a, double b)
{
  static double const rel_tol = 1e-09;
  if (a == b) return 1;
  if (isinf(a) || isinf(b)) return 0;
  double diff = fabs(b - a);
  return (diff <= fabs(rel_tol * b)) || (diff <= fabs(rel_tol * a));
}

int main(void)
{
  aye_begin();
  atexit(h3client_fini);

  struct h3client_dialer *d = h3client_dialer_new("127.0.0.1", PORT);
  aye(d);
  aye(!h3client_dialer_dial(d, h3client_deadline(1000)));

  struct h3client_stream *s = h3client_dialer_stream(d);

  struct h3c_result *result = h3c_result_new();
  aye(result);

  long deadline = h3client_deadline(1000 * 5);
  for (size_t i = 0; i < array_size(ross); ++i)
  {
    aye(!h3client_stream_put(s, cmd, ross[i].name, ross[i].seq, deadline));
  }

  for (size_t i = 0; i < array_size(ross); ++i)
  {
    h3client_stream_wait(s);
    aye(!h3client_stream_pop(s, result));
    aye(!result->errnum);
    aye(h3r_nhits(result->content) == ross[i].expect.nhits);
    if (h3r_nhits(result->content) > 0)
    {
      double lev = h3r_hit_logevalue(result->content, 0);
      aye(near(lev, ross[i].expect.ln_evalue));
    }
  }

  h3c_result_del(result);
  h3client_stream_del(s);
  h3client_dialer_del(d);

  return aye_end();
}
