#include "expect.h"
#include "lip/lip.h"
#include "rc.h"
#include "reallocf.h"
#include <string.h>

bool h3result_expect_key(struct lip_file *f, char const *key)
{
  char str[16] = {0};
  unsigned size = (unsigned)strlen(key) + 1;
  lip_read_cstr(f, size, str);

  return !lip_file_error(f) && strncmp(str, key, sizeof(str)) == 0;
}

bool h3result_expect_array_size(struct lip_file *f, unsigned size)
{
  unsigned sz = 0;
  lip_read_array_size(f, &sz);
  return !lip_file_error(f) && size == sz;
}

bool h3result_expect_map_size(struct lip_file *f, unsigned size)
{
  unsigned sz = 0;
  lip_read_map_size(f, &sz);
  return !lip_file_error(f) && size == sz;
}

int h3result_read_string(struct lip_file *f, char **str)
{
  unsigned size = 0;
  if (!lip_read_str_size(f, &size)) return H3RESULT_EUNPACK;
  if (!(*str = h3result_reallocf(*str, size + 1))) return H3RESULT_ENOMEM;
  if (!lip_read_str_data(f, size, *str)) return H3RESULT_EUNPACK;
  (*str)[size] = 0;
  return 0;
}
