#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BUFFSIZE (8 * 1024)

static bool fletcher16(FILE *fp, uint8_t *buf, size_t bufsize, long *chk);

bool file_hash(char const *filepath, long *chk)
{
  static uint8_t buffer[BUFFSIZE];
  FILE *fp = fopen(filepath, "rb");
  if (!fp) return false;

  int rc = fletcher16(fp, buffer, sizeof(buffer), chk);

  fclose(fp);
  return rc;
}

static bool fletcher16(FILE *fp, uint8_t *buf, size_t bufsize, long *chk)
{
  size_t n = 0;
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  while ((n = fread(buf, 1, bufsize, fp)) > 0)
  {
    if (n < bufsize && ferror(fp)) return false;
    for (int i = 0; i < (int)n; ++i)
    {
      sum1 = (sum1 + buf[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
    }
  }
  if (ferror(fp)) return false;

  *chk = (sum2 << 8) | sum1;
  return true;
}
