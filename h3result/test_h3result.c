#include "aye.h"
#include "h3result.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

static void test_unpack(void);
static void test_targets(void);
static void test_targets_table(void);
static void test_domains(void);
static void test_domains_table(void);

int main(void)
{
  aye_begin();
  test_unpack();
  test_targets();
  test_targets_table();
  test_domains();
  test_domains_table();
  return aye_end();
}

static struct h3r *unpack(void);
static long hash(char const *filepath);

static void test_unpack(void)
{
  int fd = open("h3result.mp", O_RDONLY);
  aye(fd);

  struct h3r *result = h3r_new();
  aye(result);

  aye(h3r_unpack(result, fd) == 0);
  aye(!close(fd));

  h3r_del(result);
}

static void test_targets(void)
{
  struct h3r *result = unpack();
  int fd = open("targets.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(fd);
  h3r_print_targets(result, fd);
  aye(!close(fd));
  aye(hash("targets.txt") == 57543L);
  h3r_del(result);
  remove("targets.txt");
}

static void test_targets_table(void)
{
  struct h3r *result = unpack();
  int fd = open("targets_table.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(fd);
  h3r_print_targets_table(result, fd);
  aye(!close(fd));
  aye(hash("targets_table.txt") == 34790L);
  h3r_del(result);
  remove("targets_table.txt");
}

static void test_domains(void)
{
  struct h3r *result = unpack();
  int fd = open("domains.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(fd);
  h3r_print_domains(result, fd);
  aye(!close(fd));
  aye(hash("domains.txt") == 46469L);
  h3r_del(result);
  remove("domains.txt");
}

static void test_domains_table(void)
{
  struct h3r *result = unpack();
  int fd = open("domains_table.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(fd);
  h3r_print_domains_table(result, fd);
  aye(!close(fd));
  aye(hash("domains_table.txt") == 3913L);
  h3r_del(result);
  remove("domains_table.txt");
}

static struct h3r *unpack(void)
{
  int fd = open("h3result.mp", O_RDONLY);
  if (!fd) return NULL;

  struct h3r *result = h3r_new();
  if (!result)
  {
    aye(!close(fd));
    return NULL;
  }

  if (h3r_unpack(result, fd))
  {
    aye(!close(fd));
    h3r_del(result);
    return NULL;
  }

  aye(!close(fd));
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
