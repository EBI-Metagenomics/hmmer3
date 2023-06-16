#include "h3result.h"
#include "vendor/minctest.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

static void test_unpack(void);
static void test_targets(void);
static void test_targets_table(void);
static void test_domains(void);
static void test_domains_table(void);

int main(void)
{
  test_unpack();
  test_targets();
  test_targets_table();
  test_domains();
  test_domains_table();
  return lfails != 0;
}

static struct h3result *unpack(void);
static long hash(char const *filepath);

static void test_unpack(void)
{
  FILE *fp = fopen("h3result.mp", "rb");
  ok(fp);

  struct h3result *result = h3result_new();
  ok(fp);

  eq(h3result_unpack(result, fp), 0);
  fclose(fp);

  h3result_del(result);
}

static void test_targets(void)
{
  struct h3result *result = unpack();
  FILE *fp = fopen("targets.txt", "wb");
  ok(fp);
  h3result_print_targets(result, fp);
  ok(!fclose(fp));
  eq(hash("targets.txt"), 57543L);
  h3result_del(result);
  remove("targets.txt");
}

static void test_targets_table(void)
{
  struct h3result *result = unpack();
  FILE *fp = fopen("targets_table.txt", "wb");
  ok(fp);
  h3result_print_targets_table(result, fp);
  ok(!fclose(fp));
  eq(hash("targets_table.txt"), 34790L);
  h3result_del(result);
  remove("targets_table.txt");
}

static void test_domains(void)
{
  struct h3result *result = unpack();
  FILE *fp = fopen("domains.txt", "wb");
  ok(fp);
  h3result_print_domains(result, fp);
  ok(!fclose(fp));
  eq(hash("domains.txt"), 46469L);
  h3result_del(result);
  // remove("domains.txt");
}

static void test_domains_table(void)
{
  struct h3result *result = unpack();
  FILE *fp = fopen("domains_table.txt", "wb");
  ok(fp);
  h3result_print_domains_table(result, fp);
  ok(!fclose(fp));
  eq(hash("domains_table.txt"), 3913L);
  h3result_del(result);
  remove("domains_table.txt");
}

static struct h3result *unpack(void)
{
  FILE *fp = fopen("h3result.mp", "rb");
  if (!fp) return NULL;

  struct h3result *result = h3result_new();
  if (!result)
  {
    fclose(fp);
    return NULL;
  }

  if (h3result_unpack(result, fp))
  {
    fclose(fp);
    h3result_del(result);
    return NULL;
  }

  fclose(fp);
  return result;
}

static long fletcher16(FILE *fp, uint8_t *buf, size_t bufsize)
{
  size_t n = 0;
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  while ((n = fread(buf, 1, bufsize, fp)) > 0)
  {
    if (n < bufsize && ferror(fp))
    {
      fprintf(stderr, "Hash failure (line %d): %s", __LINE__, strerror(errno));
      exit(1);
    }
    for (int i = 0; i < (int)n; ++i)
    {
      sum1 = (sum1 + buf[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
    }
  }
  if (ferror(fp))
  {
    fprintf(stderr, "Hash failure (line %d): %s", __LINE__, strerror(errno));
    exit(1);
  }

  return (sum2 << 8) | sum1;
}

static long hash(char const *filepath)
{
  static uint8_t buffer[8 * 1024];
  FILE *fp = fopen(filepath, "rb");
  if (!fp)
  {
    fprintf(stderr, "Failed to open file (line %d): %s", __LINE__, filepath);
    exit(1);
  }

  long chk = fletcher16(fp, buffer, sizeof(buffer));

  fclose(fp);
  return chk;
}
