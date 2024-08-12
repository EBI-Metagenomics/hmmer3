#include "expect.h"
#include "bsd.h"
#include "rc.h"
#include "read.h"
#include <string.h>

bool h3result_expect_key(struct lio_reader *f, char const *key)
{
  char str[16] = {0};
  unsigned size = (unsigned)strlen(key) + 1;
  if (read_cstring(f, size, str)) return false;
  return strncmp(str, key, sizeof(str)) == 0;
}

bool h3result_expect_array_size(struct lio_reader *f, unsigned size)
{
  unsigned sz = 0;
  if (read_array(f, &sz)) return false;
  return size == sz;
}

bool h3result_expect_map_size(struct lio_reader *f, unsigned size)
{
  unsigned sz = 0;
  if (read_map(f, &sz)) return false;
  return size == sz;
}

int h3result_read_string(struct lio_reader *f, char **str)
{
  unsigned size = 0;
  if (read_string(f, &size)) return H3RESULT_EUNPACK;
  if (!(*str = bsd_reallocf(*str, size + 1))) return H3RESULT_ENOMEM;
  if (lio_readb(f, size, (unsigned char *)*str)) return H3RESULT_EUNPACK;
  (*str)[size] = '\0';
  return 0;
}
