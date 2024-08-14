#include "aye.h"
#include "h3c_deadline.h"
#include "h3c_dialer.h"
#include "h3c_errno.h"
#include "h3c_fini.h"
#include "h3c_result.h"
#include "h3c_stream.h"
#include "helper.h"
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 51371
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

static inline int near(double a, double b)
{
  static double const rel_tol = 1e-09;
  if (a == b) return 1;
  if (isinf(a) || isinf(b)) return 0;
  double diff = fabs(b - a);
  return (diff <= fabs(rel_tol * b)) || (diff <= fabs(rel_tol * a));
}

static void test_connection_timedout(void);
static void test_open_close_connection(void);
static void test_request_timedout(void);
static void test_pack_result(void);
static void test_unpack_result(void);
static void test_print_targets(void);
static void test_print_domains(void);
static void test_print_targets_table(void);
static void test_print_domains_table(void);
static void test_reuse_results(void);
static void test_reuse_results_print(void);
static void test_reuse_connection(void);
static void test_result_api(void);

int main(void)
{
  aye_begin();
  atexit(h3client_fini);
  test_connection_timedout();
  test_open_close_connection();
  test_request_timedout();
  test_pack_result();
  test_unpack_result();
  test_print_targets();
  test_print_domains();
  test_print_targets_table();
  test_print_domains_table();
  test_reuse_results();
  test_reuse_results_print();
  test_reuse_connection();
  test_result_api();
  remove("h3result.mp");
  remove("targets.tbl");
  remove("targets.txt");
  remove("domains.tbl");
  remove("domains.txt");
  return aye_end();
}

static long earlier(void) { return h3client_deadline(-1); }
static long later(void) { return h3client_deadline(1000 * 5); }

static long hash(char const *filepath);
static struct h3client_dialer *conn_new(void);
static struct h3client_stream *conn_dial(struct h3client_dialer *d,
                                         long deadline);
static void conn_del(struct h3client_dialer *d);
static struct h3c_result *request(struct h3client_stream *s, char const *name,
                                  char const *seq, long deadline);

static void test_connection_timedout(void)
{
  struct h3client_dialer *d = conn_new();
  aye(h3client_dialer_dial(d, earlier()) == H3C_ETIMEDOUT);
  conn_del(d);
}

static void test_open_close_connection(void)
{
  struct h3client_dialer *d = conn_new();
  aye(!h3client_dialer_dial(d, later()));
  h3client_dialer_del(d);
}

static void test_request_timedout(void)
{
  struct h3client_dialer *d = conn_new();
  struct h3client_stream *s = conn_dial(d, later());

  aye(!h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, earlier()));
  h3client_stream_wait(s);
  struct h3c_result *result = h3c_result_new();
  aye(result);
  aye(h3client_stream_pop(s, result) == H3C_ETIMEDOUT);

  h3c_result_del(result);
  h3client_stream_del(s);
  conn_del(d);
}

static struct h3c_result *request_ross(void)
{
  struct h3client_dialer *d = conn_new();
  aye(d);
  struct h3client_stream *s = conn_dial(d, later());
  aye(s);
  struct h3c_result *r = request(s, ross[GOOD].name, ross[GOOD].seq, later());
  aye(r);

  h3client_stream_del(s);
  conn_del(d);
  return r;
}

static void test_pack_result(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  int file = open("h3result.mp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  aye(!h3r_pack(result->content, file));
  aye(!close(file));

  printf("_%d: %ld\n", __LINE__, hash("h3result.mp"));
  aye(hash("h3result.mp") == 8392L);

  h3c_result_del(result);
}

static void test_unpack_result(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  int file = open("h3result.mp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  aye(!h3r_pack(result->content, file));
  aye(!close(file));

  printf("_%d: %ld\n", __LINE__, hash("h3result.mp"));
  aye(hash("h3result.mp") == 8392L);

  file = open("h3result.mp", O_RDONLY);
  aye(file >= 0);
  aye(h3r_unpack(result->content, file) == 0);
  aye(!close(file));

  file = open("h3result.mp", O_RDONLY);
  aye(file >= 0);
  aye(!h3r_unpack(result->content, file));
  aye(!close(file));

  printf("_%d: %ld\n", __LINE__, hash("h3result.mp"));
  aye(hash("h3result.mp") == 8392L);

  h3c_result_del(result);
}

static void test_print_targets(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  int file = open("targets.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  h3r_print_targets(result->content, file);
  aye(!close(file));

  aye(hash("targets.txt") == 57543L);

  h3c_result_del(result);
}

static void test_print_domains(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  int file = open("domains.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  h3r_print_domains(result->content, file);
  aye(!close(file));

  aye(hash("domains.txt") == 46469L);

  h3c_result_del(result);
}

static void test_print_targets_table(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  int file = open("targets.tbl", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  h3r_print_targets_table(result->content, file);
  aye(!close(file));

  aye(hash("targets.tbl") == 34790L);

  h3c_result_del(result);
}

static void test_print_domains_table(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  int file = open("domains.tbl", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  h3r_print_domains_table(result->content, file);
  aye(!close(file));

  aye(hash("domains.tbl") == 3913L);

  h3c_result_del(result);
}

static void test_reuse_results(void)
{
  struct h3c_result *result = h3c_result_new();
  aye(result);

  struct h3client_dialer *d = conn_new();
  aye(!h3client_dialer_dial(d, later()));

  struct h3client_stream *s = h3client_dialer_stream(d);

  aye(!h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, later()));
  h3client_stream_wait(s);
  aye(!h3client_stream_pop(s, result));

  h3client_stream_del(s);
  h3client_dialer_del(d);

  h3c_result_del(result);
}

static void test_reuse_results_print(void)
{
  int64_t target = 57543L;
  int64_t domain = 46469L;

  struct h3c_result *result = h3c_result_new();
  aye(result);

  struct h3client_dialer *d = conn_new();
  aye(!h3client_dialer_dial(d, later()));

  struct h3client_stream *s = h3client_dialer_stream(d);

  aye(!h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, later()));
  h3client_stream_wait(s);
  aye(!h3client_stream_pop(s, result));

  h3client_stream_del(s);
  h3client_dialer_del(d);

  int file = open("targets.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  h3r_print_targets(result->content, file);
  aye(!close(file));
  aye(hash("targets.txt") == target);

  file = open("domains.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  h3r_print_domains(result->content, file);
  aye(!close(file));
  aye(hash("domains.txt") == domain);

  h3c_result_del(result);
}

static void test_reuse_connection(void)
{
  struct h3c_result *result = h3c_result_new();
  aye(result);

  struct h3client_dialer *d = conn_new();
  aye(!h3client_dialer_dial(d, later()));

  struct h3client_stream *s = h3client_dialer_stream(d);

  aye(!h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, later()));
  h3client_stream_wait(s);
  aye(!h3client_stream_pop(s, result));

  aye(!h3client_stream_put(s, cmd, ross[BAD].name, ross[BAD].seq, later()));
  h3client_stream_wait(s);
  aye(!h3client_stream_pop(s, result));

  h3client_stream_del(s);
  h3client_dialer_del(d);

  h3c_result_del(result);
}

static void test_result_api(void)
{
  struct h3c_result *result = request_ross();
  aye(result);

  char const *name[] = {"000000005", "000000003", "000000002", "000000004"};
  char const *acc[] = {"PF13460.8", "PF01370.23", "PF01073.21", "PF05368.15"};
  double eln[] = {-53.808984215028, -38.604817925966, -34.047928969184,
                  -31.743561848164};

  aye(h3r_nhits(result->content) == 4);
  aye(!result->errnum);

  for (int i = 0; i < 4; ++i)
  {
    aye(!strcmp(name[i], h3r_hit_name(result->content, i)));
    aye(!strcmp(acc[i], h3r_hit_accession(result->content, i)));
    aye(near(eln[i], h3r_hit_logevalue(result->content, i)));
  }

  h3c_result_del(result);
}

static struct h3client_dialer *conn_new(void)
{
  struct h3client_dialer *d = h3client_dialer_new("127.0.0.1", PORT);
  aye(d);
  return d;
}

static struct h3client_stream *conn_dial(struct h3client_dialer *d,
                                         long deadline)
{
  aye(!h3client_dialer_dial(d, deadline));
  return h3client_dialer_stream(d);
}

static void conn_del(struct h3client_dialer *d) { h3client_dialer_del(d); }

static struct h3c_result *request(struct h3client_stream *s, char const *name,
                                  char const *seq, long deadline)
{
  aye(!h3client_stream_put(s, cmd, name, seq, deadline));
  h3client_stream_wait(s);
  struct h3c_result *result = h3c_result_new();
  aye(result);
  aye(!h3client_stream_pop(s, result));
  return result;
}

static long fletcher16(int fd, uint8_t *buf, size_t bufsize)
{
  ssize_t n = 0;
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  while ((n = read(fd, buf, bufsize)) > 0)
  {
    for (int i = 0; i < (int)n; ++i)
    {
      sum1 = (sum1 + buf[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
    }
  }
  if (n < 0)
  {
    fprintf(stderr, "Hash failure (line %d)", __LINE__);
    exit(1);
  }

  return (sum2 << 8) | sum1;
}

static long hash(char const *filepath)
{
  static uint8_t buffer[8 * 1024];
  int fd = open(filepath, O_RDONLY);
  if (!fd)
  {
    fprintf(stderr, "Failed to open file (line %d): %s", __LINE__, filepath);
    exit(1);
  }

  long chk = fletcher16(fd, buffer, sizeof(buffer));

  aye(!close(fd));
  return chk;
}
