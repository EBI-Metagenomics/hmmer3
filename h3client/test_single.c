#include "file_hash.h"
#include "fs.h"
#include "h3client/h3client.h"
#include "helper.h"
#include "hope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51371
static char const cmd[] = "--hmmdb 1 --acc --cut_ga";

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
  return EXIT_SUCCESS;
}

static long earlier(void) { return h3client_deadline(-1); }
static long later(void) { return h3client_deadline(1000 * 5); }

static long hash(char const *filepath);
static struct h3client_dialer *conn_new(void);
static struct h3client_stream *conn_dial(struct h3client_dialer *d,
                                         long deadline);
static void conn_del(struct h3client_dialer *d);
static struct h3r *request(struct h3client_stream *s,
                                       char const *name, char const *seq,
                                       long deadline);

static void test_connection_timedout(void)
{
  struct h3client_dialer *d = conn_new();
  eq(h3client_dialer_dial(d, earlier()), H3CLIENT_ETIMEDOUT);
  conn_del(d);
}

static void test_open_close_connection(void)
{
  struct h3client_dialer *d = conn_new();
  eq(h3client_dialer_dial(d, later()), 0);
  h3client_dialer_del(d);
}

static void test_request_timedout(void)
{
  struct h3client_dialer *d = conn_new();
  struct h3client_stream *s = conn_dial(d, later());

  eq(h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, earlier()),
     0);
  h3client_stream_wait(s);
  struct h3r *result = h3client_result_new();
  notnull(result);
  eq(h3client_stream_pop(s, result), H3CLIENT_ETIMEDOUT);

  h3client_result_del(result);
  h3client_stream_del(s);
  conn_del(d);
}

static struct h3r *request_ross(void)
{
  struct h3client_dialer *d = conn_new();
  struct h3client_stream *s = conn_dial(d, later());
  struct h3r *r =
      request(s, ross[GOOD].name, ross[GOOD].seq, later());

  h3client_stream_del(s);
  conn_del(d);
  return r;
}

static void test_pack_result(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  FILE *file = fopen(TMPDIR "/h3result.mp", "wb");
  notnull(file);
  eq(h3client_result_pack(result, file), 0);
  fclose(file);

  eq(hash(TMPDIR "/h3result.mp"), 63792L);

  h3client_result_del(result);
}

static void test_unpack_result(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  FILE *file = fopen(TMPDIR "/h3result.mp", "wb");
  notnull(file);
  eq(h3client_result_pack(result, file), 0);
  fclose(file);

  eq(hash(TMPDIR "/h3result.mp"), 63792L);

  file = fopen(TMPDIR "/h3result.mp", "rb");
  notnull(file);
  eq(h3client_result_unpack(result, file), 0);
  fclose(file);

  file = fopen(TMPDIR "/h3result.mp", "wb");
  notnull(file);
  eq(h3client_result_pack(result, file), 0);
  fclose(file);

  eq(hash(TMPDIR "/h3result.mp"), 63792L);

  h3client_result_del(result);
}

static void test_print_targets(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  FILE *file = fopen(TMPDIR "/targets.txt", "wb");
  notnull(file);
  h3client_result_print_targets(result, file);
  fclose(file);

  eq(hash(TMPDIR "/targets.txt"), 57543L);

  h3client_result_del(result);
}

static void test_print_domains(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  FILE *file = fopen(TMPDIR "/domains.txt", "wb");
  notnull(file);
  h3client_result_print_domains(result, file);
  fclose(file);

  eq(hash(TMPDIR "/domains.txt"), 46469L);

  h3client_result_del(result);
}

static void test_print_targets_table(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  FILE *file = fopen(TMPDIR "/targets.tbl", "wb");
  notnull(file);
  h3client_result_print_targets_table(result, file);
  fclose(file);

  eq(hash(TMPDIR "/targets.tbl"), 34790L);

  h3client_result_del(result);
}

static void test_print_domains_table(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  FILE *file = fopen(TMPDIR "/domains.tbl", "wb");
  notnull(file);
  h3client_result_print_domains_table(result, file);
  fclose(file);

  eq(hash(TMPDIR "/domains.tbl"), 3913L);

  h3client_result_del(result);
}

static void test_reuse_results(void)
{
  struct h3r *result = h3client_result_new();
  notnull(result);

  struct h3client_dialer *d = conn_new();
  eq(h3client_dialer_dial(d, later()), 0);

  struct h3client_stream *s = h3client_dialer_stream(d);

  eq(h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, later()), 0);
  h3client_stream_wait(s);
  eq(h3client_stream_pop(s, result), 0);

  h3client_stream_del(s);
  h3client_dialer_del(d);

  h3client_result_del(result);
}

static void test_reuse_results_print(void)
{
  int64_t target = 57543L;
  int64_t domain = 46469L;

  struct h3r *result = h3client_result_new();
  notnull(result);

  struct h3client_dialer *d = conn_new();
  eq(h3client_dialer_dial(d, later()), 0);

  struct h3client_stream *s = h3client_dialer_stream(d);

  eq(h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, later()), 0);
  h3client_stream_wait(s);
  eq(h3client_stream_pop(s, result), 0);

  h3client_stream_del(s);
  h3client_dialer_del(d);

  FILE *file = fopen(TMPDIR "/targets.txt", "wb");
  notnull(file);
  h3client_result_print_targets(result, file);
  fclose(file);
  eq(hash(TMPDIR "/targets.txt"), target);

  file = fopen(TMPDIR "/domains.txt", "wb");
  notnull(file);
  h3client_result_print_domains(result, file);
  fclose(file);
  eq(hash(TMPDIR "/domains.txt"), domain);

  h3client_result_del(result);
}

static void test_reuse_connection(void)
{
  struct h3r *result = h3client_result_new();
  notnull(result);

  struct h3client_dialer *d = conn_new();
  eq(h3client_dialer_dial(d, later()), 0);

  struct h3client_stream *s = h3client_dialer_stream(d);

  eq(h3client_stream_put(s, cmd, ross[GOOD].name, ross[GOOD].seq, later()), 0);
  h3client_stream_wait(s);
  eq(h3client_stream_pop(s, result), 0);

  eq(h3client_stream_put(s, cmd, ross[BAD].name, ross[BAD].seq, later()), 0);
  h3client_stream_wait(s);
  eq(h3client_stream_pop(s, result), 0);

  h3client_stream_del(s);
  h3client_dialer_del(d);

  h3client_result_del(result);
}

static void test_result_api(void)
{
  struct h3r *result = request_ross();
  notnull(result);

  char const *name[] = {"000000005", "000000003", "000000002", "000000004"};
  char const *acc[] = {"PF13460.8", "PF01370.23", "PF01073.21", "PF05368.15"};
  double eln[] = {-53.808984215028, -38.604817925966, -34.047928969184,
                  -31.743561848164};

  eq(h3client_result_nhits(result), 4);
  eq(h3client_result_errnum(result), 0);

  for (int i = 0; i < 4; ++i)
  {
    eq(name[i], h3client_result_hit_name(result, i));
    eq(acc[i], h3client_result_hit_acc(result, i));
    close(eln[i], h3client_result_hit_evalue_ln(result, i));
  }

  h3client_result_del(result);
}

static long hash(char const *filepath)
{
  long actual = 0;
  cond(file_hash(filepath, &actual));
  return actual;
}

static struct h3client_dialer *conn_new(void)
{
  struct h3client_dialer *d = h3client_dialer_new("127.0.0.1", PORT);
  notnull(d);
  return d;
}

static struct h3client_stream *conn_dial(struct h3client_dialer *d,
                                         long deadline)
{
  eq(h3client_dialer_dial(d, deadline), 0);
  return h3client_dialer_stream(d);
}

static void conn_del(struct h3client_dialer *d) { h3client_dialer_del(d); }

static struct h3r *request(struct h3client_stream *s,
                                       char const *name, char const *seq,
                                       long deadline)
{
  eq(h3client_stream_put(s, cmd, name, seq, deadline), 0);
  h3client_stream_wait(s);
  struct h3r *result = h3client_result_new();
  notnull(result);
  eq(h3client_stream_pop(s, result), 0);
  return result;
}
